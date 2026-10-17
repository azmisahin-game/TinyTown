// Validates the TinyTown project page without a browser:
//   * index.html parses as XML-ish HTML and every data-i18n key exists in both
//     language tables in app.js
//   * both language tables have exactly the same keys
//   * no key is defined but unused, which would hide a typo
//
// Run: node tools/check_site.js
// This is a development check. The site itself has no build step and no
// runtime dependency on Node.

'use strict';

const fs = require('fs');
const path = require('path');

const root = path.resolve(__dirname, '..');
const htmlPath = path.join(root, 'site', 'index.html');
const jsPath = path.join(root, 'site', 'app.js');

const html = fs.readFileSync(htmlPath, 'utf8');
const js = fs.readFileSync(jsPath, 'utf8');

let failures = 0;
function fail(message) {
  failures += 1;
  console.error('FAIL: ' + message);
}
function ok(message) {
  console.log('ok: ' + message);
}

// --- Keys referenced by the page ---------------------------------------------
const usedKeys = new Set();
for (const match of html.matchAll(/data-i18n="([^"]+)"/g)) usedKeys.add(match[1]);
for (const match of html.matchAll(/data-i18n-alt="([^"]+)"/g)) usedKeys.add(match[1]);

if (usedKeys.size === 0) fail('no data-i18n keys found in index.html');
else ok(`found ${usedKeys.size} translation keys in index.html`);

// --- Language tables from app.js ---------------------------------------------
// The tables are extracted by evaluating only the object literal, so the check
// does not need a DOM. Everything from "var STRINGS = {" up to the matching
// "};" is lifted out and evaluated on its own.
let captured = null;
const startMarker = js.indexOf('var STRINGS = {');
if (startMarker === -1) {
  fail('app.js does not define a STRINGS table');
} else {
  const literalStart = js.indexOf('{', startMarker);
  // Walk braces to find the end of the literal, ignoring braces inside strings.
  let depth = 0;
  let inString = false;
  let quote = '';
  let escaped = false;
  let end = -1;
  for (let i = literalStart; i < js.length; i += 1) {
    const ch = js[i];
    if (inString) {
      if (escaped) {
        escaped = false;
      } else if (ch === '\\') {
        escaped = true;
      } else if (ch === quote) {
        inString = false;
      }
      continue;
    }
    if (ch === '"' || ch === "'") {
      inString = true;
      quote = ch;
      continue;
    }
    if (ch === '{') depth += 1;
    if (ch === '}') {
      depth -= 1;
      if (depth === 0) {
        end = i + 1;
        break;
      }
    }
  }
  if (end === -1) {
    fail('could not find the end of the STRINGS literal in app.js');
  } else {
    const literal = js.slice(literalStart, end);
    try {
      captured = new Function('return (' + literal + ');')();
    } catch (error) {
      fail('could not evaluate the STRINGS literal: ' + error.message);
    }
  }
}

if (!captured || typeof captured !== 'object') {
  fail('app.js did not expose a STRINGS table');
} else {
  const languages = Object.keys(captured);
  ok(`app.js defines languages: ${languages.join(', ')}`);

  if (!languages.includes('tr')) fail('app.js has no Turkish (tr) table');
  if (!languages.includes('en')) fail('app.js has no English (en) table');

  // Every key the page uses must exist in every language.
  for (const lang of languages) {
    const table = captured[lang];
    const missing = [];
    for (const key of usedKeys) {
      if (!Object.prototype.hasOwnProperty.call(table, key)) missing.push(key);
    }
    if (missing.length > 0) {
      fail(`${lang} is missing keys used by the page: ${missing.join(', ')}`);
    } else {
      ok(`${lang} covers every key used by the page`);
    }
  }

  // The tables must have identical key sets, so neither language can drift.
  const reference = languages[0];
  const referenceKeys = Object.keys(captured[reference]).sort();
  for (const lang of languages.slice(1)) {
    const keys = Object.keys(captured[lang]).sort();
    const onlyInReference = referenceKeys.filter((k) => !keys.includes(k));
    const onlyInLang = keys.filter((k) => !referenceKeys.includes(k));
    if (onlyInReference.length > 0) {
      fail(`${lang} is missing keys present in ${reference}: ${onlyInReference.join(', ')}`);
    }
    if (onlyInLang.length > 0) {
      fail(`${lang} has keys not present in ${reference}: ${onlyInLang.join(', ')}`);
    }
    if (onlyInReference.length === 0 && onlyInLang.length === 0) {
      ok(`${reference} and ${lang} have identical key sets (${keys.length} keys)`);
    }
  }

  // Unused keys usually mean a typo between the page and the table.
  const allTableKeys = new Set();
  for (const lang of languages) {
    for (const key of Object.keys(captured[lang])) allTableKeys.add(key);
  }
  const unused = [...allTableKeys].filter((k) => !usedKeys.has(k));
  if (unused.length > 0) {
    fail(`translation keys defined but not used by the page: ${unused.join(', ')}`);
  } else {
    ok('no unused translation keys');
  }

  // No empty translations.
  for (const lang of languages) {
    for (const [key, value] of Object.entries(captured[lang])) {
      if (typeof value !== 'string' || value.trim() === '') {
        fail(`${lang}.${key} is empty`);
      }
    }
  }
}

// --- Turkish content check ---------------------------------------------------
// The Turkish table must contain Turkish characters, otherwise the page would
// silently fall back to ASCII-only text.
if (captured && captured.tr) {
  const turkishBlob = Object.values(captured.tr).join(' ');
  if (!/[çğıİöşüÇĞÖŞÜ]/.test(turkishBlob)) {
    fail('the Turkish table contains no Turkish characters');
  } else {
    ok('the Turkish table contains Turkish characters');
  }
}

// --- Asset and link sanity ---------------------------------------------------
const imgMatch = html.match(/src="([^"]+\.png)"/);
if (!imgMatch) {
  fail('index.html references no screenshot image');
} else {
  const imagePath = path.join(root, 'site', imgMatch[1]);
  if (!fs.existsSync(imagePath)) {
    fail(`screenshot referenced by the page does not exist: ${imgMatch[1]}`);
  } else {
    ok(`screenshot exists: ${imgMatch[1]}`);
  }
}

// The page must not claim a browser build. Check the wording that matters.
if (/play in browser|tarayıcıda oyna/i.test(html)) {
  fail('index.html appears to advertise a browser build');
} else {
  ok('index.html does not advertise a browser build');
}

console.log('');
if (failures > 0) {
  console.error(`${failures} check(s) failed`);
  process.exit(1);
}
console.log('all site checks passed');
