#include "core/pathfinding.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

namespace tt {
namespace {

constexpr float kDiagCost = 1.41421356f;
constexpr float kEps = 1e-4f;

struct OpenNode {
    float f = 0.0f;
    int index = 0;
    int order = 0;
};

// min-heap ordered by f, then by insertion order for deterministic results.
struct OpenCompare {
    bool operator()(const OpenNode& a, const OpenNode& b) const {
        if (std::fabs(a.f - b.f) > kEps) return a.f > b.f;
        return a.order > b.order;
    }
};

int cellOf(const World& world, int ix, int iz) { return iz * world.gridWidth + ix; }

bool freeCell(const World& world, int ix, int iz) {
    if (!insideGrid(world, ix, iz)) return false;
    return world.walkable[static_cast<size_t>(cellOf(world, ix, iz))] != 0;
}

float heuristic(int ax, int az, int bx, int bz) {
    const float dx = static_cast<float>(std::abs(ax - bx));
    const float dz = static_cast<float>(std::abs(az - bz));
    // Octile distance: admissible for 8-connected movement with these costs.
    return (dx + dz) + (kDiagCost - 2.0f) * std::min(dx, dz);
}

}  // namespace

bool hasLineOfSight(const World& world, Vec2 a, Vec2 b) {
    if (isBlocked(world, a) || isBlocked(world, b)) return false;

    const float dist = distance(a, b);
    const float step = std::max(0.25f, world.cellSize * 0.4f);
    const int steps = std::max(1, static_cast<int>(std::ceil(dist / step)));
    for (int i = 0; i <= steps; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(steps);
        const Vec2 p{a.x + (b.x - a.x) * t, a.z + (b.z - a.z) * t};
        if (isBlocked(world, p)) return false;
    }
    return true;
}

PathResult findPath(const World& world, Vec2 start, Vec2 goal) {
    PathResult result;

    if (!isWalkable(world, start) || !isWalkable(world, goal)) {
        return result;  // explicit "no route": caller decides how to fail
    }

    const int sx = gridCellX(world, start);
    const int sz = gridCellZ(world, start);
    const int gx = gridCellX(world, goal);
    const int gz = gridCellZ(world, goal);

    if (sx == gx && sz == gz) {
        result.found = true;
        result.points = {start, goal};
        return result;
    }

    const int total = world.gridWidth * world.gridHeight;
    const float inf = std::numeric_limits<float>::infinity();

    std::vector<float> gScore(static_cast<size_t>(total), inf);
    std::vector<int> cameFrom(static_cast<size_t>(total), -1);
    std::vector<uint8_t> closed(static_cast<size_t>(total), 0);

    std::priority_queue<OpenNode, std::vector<OpenNode>, OpenCompare> open;
    int order = 0;

    const int startIndex = cellOf(world, sx, sz);
    gScore[static_cast<size_t>(startIndex)] = 0.0f;
    open.push({heuristic(sx, sz, gx, gz), startIndex, order++});

    const int goalIndex = cellOf(world, gx, gz);

    static constexpr int kDx[8] = {1, -1, 0, 0, 1, 1, -1, -1};
    static constexpr int kDz[8] = {0, 0, 1, -1, 1, -1, 1, -1};

    bool reached = false;
    while (!open.empty()) {
        const OpenNode current = open.top();
        open.pop();

        if (closed[static_cast<size_t>(current.index)]) continue;
        closed[static_cast<size_t>(current.index)] = 1;
        ++result.cellsExplored;

        if (current.index == goalIndex) {
            reached = true;
            break;
        }

        const int cx = current.index % world.gridWidth;
        const int cz = current.index / world.gridWidth;

        for (int d = 0; d < 8; ++d) {
            const int nx = cx + kDx[d];
            const int nz = cz + kDz[d];
            if (!freeCell(world, nx, nz)) continue;

            const bool diagonal = kDx[d] != 0 && kDz[d] != 0;
            if (diagonal) {
                // Do not cut corners through a blocked orthogonal neighbour.
                if (!freeCell(world, cx + kDx[d], cz) || !freeCell(world, cx, cz + kDz[d])) {
                    continue;
                }
            }

            const int nIndex = cellOf(world, nx, nz);
            if (closed[static_cast<size_t>(nIndex)]) continue;

            const float step = diagonal ? kDiagCost : 1.0f;
            const float tentative = gScore[static_cast<size_t>(current.index)] + step;
            if (tentative + kEps < gScore[static_cast<size_t>(nIndex)]) {
                gScore[static_cast<size_t>(nIndex)] = tentative;
                cameFrom[static_cast<size_t>(nIndex)] = current.index;
                open.push({tentative + heuristic(nx, nz, gx, gz), nIndex, order++});
            }
        }
    }

    if (!reached) return result;

    // Reconstruct cell centres from goal back to start.
    std::vector<Vec2> raw;
    int node = goalIndex;
    while (node != -1) {
        const int ix = node % world.gridWidth;
        const int iz = node / world.gridWidth;
        raw.push_back(gridCellCenter(world, ix, iz));
        if (node == startIndex) break;
        node = cameFrom[static_cast<size_t>(node)];
    }
    std::reverse(raw.begin(), raw.end());

    // Anchor the polyline to the exact start/goal the caller asked for.
    raw.front() = start;
    raw.back() = goal;

    // Line-of-sight smoothing: keep only waypoints that are needed.
    std::vector<Vec2> smoothed;
    smoothed.push_back(raw.front());
    size_t anchor = 0;
    while (anchor + 1 < raw.size()) {
        size_t best = anchor + 1;
        for (size_t probe = raw.size() - 1; probe > anchor; --probe) {
            if (hasLineOfSight(world, raw[anchor], raw[probe])) {
                best = probe;
                break;
            }
        }
        smoothed.push_back(raw[best]);
        anchor = best;
    }

    result.found = true;
    result.points = std::move(smoothed);
    return result;
}

}  // namespace tt
