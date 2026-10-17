// TinyTown project page: language switching only.
//
// No framework, no build step. Turkish is the default; English is a full
// translation in the table below. The chosen language is remembered in
// localStorage, and the page falls back to Turkish when storage is unavailable
// (for example in a private window or with site data blocked).

(function () {
  'use strict';

  var STRINGS = {
    tr: {
      'skip': 'İçeriğe geç',
      'nav.about': 'Nedir',
      'nav.play': 'Nasıl oynanır',
      'nav.agents': 'Ajanlar',
      'nav.download': 'İndir',
      'nav.roadmap': 'Yol haritası',

      'hero.eyebrow': 'C++ · SDL2 · OpenGL · oyun motoru yok',
      'hero.title': 'Kodla üretilen küçük bir dünya',
      'hero.lead': 'TinyTown, prosedürel olarak üretilen küçük bir 3B şehir ve bu şehirde basit görevler yapan görünür ajanlardan oluşur. Harici görsel varlık, oyun motoru veya zorunlu yapay zekâ modeli kullanmaz.',
      'hero.cta': 'İndir',
      'hero.source': 'Kaynak kodu',
      'shot.alt': 'TinyTown v0.1 çalışırken: şehir, üç ajan ve inceleme paneli',
      'shot.caption': 'Uygulamanın kendisi tarafından üretilmiş gerçek ekran görüntüsü. Tasarım görseli değildir.',

      'about.title': 'TinyTown nedir',
      'about.p1': 'TinyTown, küçük ama gerçek bir ürünün ne kadarını doğrudan kodla, az ve gerekçeli bağımlılıkla yapabileceğimizi gösteren küçük bir açık kaynak deneyidir. Şehirdeki her şekil, her renk ve yazı tipi kodla üretilir.',
      'about.p2': 'Proje tek bir Win32/OpenGL dosyası olarak başladı. O dosya reference/win32-minicity/main.cpp yolunda tarih olarak saklanır; Windows\'a özel olduğu için Linux veya macOS\'ta olduğu gibi derlenemez. TinyTown onun amacını korur ve taşınabilir bir kabuk üzerine yeniden kurar.',
      'about.fact1': 'Tek bağımlılık: pencere, girdi ve OpenGL bağlamı için SDL2 2.32.4.',
      'about.fact2': 'Çevrimdışı: derleme sonrası ağ bağlantısı, hesap veya API anahtarı gerekmez.',
      'about.fact3': 'Deterministik: aynı tohum her zaman aynı şehri ve aynı ajan davranışını üretir.',
      'about.fact4': 'Test edilebilir çekirdek: simülasyon, pencere açmadan 77 testle doğrulanır.',

      'play.title': 'Nasıl oynanır',
      'play.intro': 'Uygulamayı açın, şehirde gezin, ajanları izleyin ve birini seçip ne yaptığını okuyun.',
      'keys.move': 'Hareket',
      'keys.sprint': 'Hızlı koş',
      'keys.mouse': 'İmleci al / bırak, sonra fare ile bak',
      'keys.view': 'Genel / sokak görünümü',
      'keys.select': 'Ajan seç',
      'keys.pause': 'Duraklat / devam',
      'keys.reset': 'Dünyayı tohuma sıfırla',
      'keys.lang': 'Dili değiştir (TR / EN)',
      'keys.shot': 'Ekran görüntüsü kaydet',
      'keys.help': 'Yardım katmanı',
      'keys.quit': 'İmleci bırak, tekrar basınca çık',
      'play.note': 'Fare ile bakış isteğe bağlıdır. İmleç kilitlenmez; M ile açılıp kapanır ve Esc her zaman serbest bırakır.',

      'agents.title': 'Üç deterministik ajan',
      'agents.intro': 'Ajanlar belirlenmiş kurallarla çalışan görev durum makineleridir. Karar vermek için bir dil modeli kullanmazlar. Durumlar: Bekliyor → Hedefe gidiyor → Görevde → Tamamlandı veya Başarısız.',
      'agents.patrol.title': 'Devriye',
      'agents.patrol.body': 'Kaldırım köşelerinden oluşan bir döngüde yürür ve turu tekrarlar.',
      'agents.deliver.title': 'Teslimat',
      'agents.deliver.body': 'Depodan paket alır, bir dükkânın önüne götürür ve depoya döner. Taşınan paket ajanın üstünde çizilir.',
      'agents.survey.title': 'İnceleme',
      'agents.survey.body': 'Bir noktaya gider, orada kısa süre çalışır ve sonucu panele yazar.',
      'agents.note': 'Yol bulma, binaların içinden geçmez. Ulaşılamayan hedefte ajan uydurmak yerine "yol yok" diye açıkça başarısız olur ve yeni görev alır.',

      'download.title': 'İndir',
      'download.intro': 'Kaynak kodu derlemeniz gerekmez. Paketler GitHub Release üzerinden yayımlanır.',
      'download.win.file': 'TinyTown-v0.1.0-windows-x64.zip',
      'download.win.body': 'ZIP\'i çıkarın ve tinytown.exe dosyasına çift tıklayın. SDL2 kütüphanesi paketin içindedir.',
      'download.linux.file': 'TinyTown-v0.1.0-linux-x64.tar.gz',
      'download.linux.body': 'Arşivi çıkarıp ./tinytown çalıştırın. SDL2 ve OpenGL sistem bağımlılığıdır, pakette gelmez.',
      'download.cta': 'GitHub Release sayfası',
      'download.source': 'Kaynak kodu ve testler',
      'download.note': 'İndirme, bilgisayarınızdaki kurulu bir kopyayı otomatik güncellemez. Otomatik güncelleme bu sürümde yoktur.',

      'honest.title': 'Bu sürüm ne yapmaz',
      'honest.intro': 'Yanlış beklenti oluşturmamak için açıkça yazıyoruz. Aşağıdakiler v0.1.0 kapsamında yoktur:',
      'honest.li1': 'LLM, öğrenen yapay zekâ, TTS veya yapay zekâ görüntü üretimi.',
      'honest.li2': 'Ses, WAV dışa aktarımı, MP4 veya animasyon karesi dizisi.',
      'honest.li3': 'Tarayıcı / WebAssembly sürümü.',
      'honest.li4': 'Otomatik güncelleme.',
      'honest.browser': 'Uygulama yerel bir SDL2 penceresi ve compatibility-profile immediate-mode OpenGL kullanır. Bu hâliyle tarayıcıda çalışmaz; tarayıcı hedefi ayrı bir kilometre taşıdır ve gerçek bir WebGL/GLES renderer ile Emscripten derlemesi gerektirir.',

      'status.title': 'Durum',
      'status.caption': 'Platform doğrulama durumu',
      'status.th.platform': 'Platform',
      'status.th.build': 'Derleme',
      'status.th.tests': 'Testler',
      'status.th.gui': 'Arayüz açıldı',
      'status.yes': 'Evet',
      'status.no': 'Hayır',
      'status.tests77': '77 / 77',
      'status.linuxGui': 'Evet, Xvfb + Mesa ile',
      'status.winGui': 'Hayır, runner\'da ekran yok',
      'status.browser': 'Tarayıcı',
      'status.note': 'Windows\'ta uygulama derlenir ve testler geçer, ancak etkileşimli pencere ve OpenGL çizimi CI runner\'ında denenmedi. Bu, gerçek masaüstü denemesinin yerini tutmaz.',

      'roadmap.title': 'Yol haritası',
      'roadmap.intro': 'Sonraki adımlar ayrı ve ölçülebilir görevlerdir. Hiçbiri v0.1.0\'ın parçası değildir ve tamamlanmış gibi gösterilmez.',
      'roadmap.v010.title': 'v0.1.0 — Yaşayan dünya ve yayımlanabilir temel',
      'roadmap.v010.body': 'Tamamlandı: prosedürel şehir, gezilebilir kamera, üç deterministik ajan, Türkçe/İngilizce arayüz, testler, paketler, Release ve bu site.',
      'roadmap.v020.title': 'v0.2 — Üretim ve paylaşılabilir medya çıktıları',
      'roadmap.v020.body': 'Planlandı, henüz başlamadı: deterministik animasyon karesi dizisi, kodla sentezlenen ortam sesi ve ses düzeyi kontrolü, WAV dışa aktarımı, isteğe bağlı FFmpeg ile MP4.',
      'roadmap.v030.title': 'v0.3 — Daha geniş erişim ve tarayıcı değerlendirmesi',
      'roadmap.v030.body': 'Planlandı, henüz başlamadı: yerel Windows/Linux paketlerinin yeniden doğrulanması, WebGL/GLES uyumlu renderer fizibilitesi ve çalışan bir tarayıcı prototipi. Çalışan bir prototip yayımlanmadan "tarayıcı sürümü var" denmeyecek.',

      'footer.license': 'TinyTown, MIT lisansı ile yayımlanır. SDL2 (zlib lisansı) tek harici bağımlılıktır.',
      'footer.repo': 'Depo',
      'footer.notices': 'Üçüncü taraf bildirimleri',
      'footer.roadmap': 'Yol haritası'
    },

    en: {
      'skip': 'Skip to content',
      'nav.about': 'What it is',
      'nav.play': 'How to play',
      'nav.agents': 'Agents',
      'nav.download': 'Download',
      'nav.roadmap': 'Roadmap',

      'hero.eyebrow': 'C++ · SDL2 · OpenGL · no game engine',
      'hero.title': 'A tiny world built from code',
      'hero.lead': 'TinyTown is a procedurally generated miniature 3D town with visible agents doing simple tasks inside it. It uses no imported art assets, no game engine, and no mandatory AI model.',
      'hero.cta': 'Download',
      'hero.source': 'Source code',
      'shot.alt': 'TinyTown v0.1 running: the town, three agents, and the inspect panel',
      'shot.caption': 'A real screenshot produced by the application itself. Not a concept image.',

      'about.title': 'What TinyTown is',
      'about.p1': 'TinyTown is a small open-source experiment that shows how much of a small but real product can be built directly in code, with few and justified dependencies. Every shape, colour, and the bitmap font are generated by code.',
      'about.p2': 'The project began as a single Win32/OpenGL file. That file is kept as history at reference/win32-minicity/main.cpp; it is Windows-only and cannot be compiled on Linux or macOS as written. TinyTown keeps its intent and rebuilds it on a portable shell.',
      'about.fact1': 'One dependency: SDL2 2.32.4 for the window, input, and the OpenGL context.',
      'about.fact2': 'Offline: no network connection, account, or API key is needed after the build.',
      'about.fact3': 'Deterministic: the same seed always produces the same town and the same agent behaviour.',
      'about.fact4': 'Testable core: the simulation is verified by 77 tests without opening a window.',

      'play.title': 'How to play',
      'play.intro': 'Open the application, walk around the town, watch the agents, and select one to read what it is doing.',
      'keys.move': 'Move',
      'keys.sprint': 'Sprint',
      'keys.mouse': 'Capture / release the cursor, then look with the mouse',
      'keys.view': 'Overview / street view',
      'keys.select': 'Select agent',
      'keys.pause': 'Pause / resume',
      'keys.reset': 'Reset the world to its seed',
      'keys.lang': 'Switch language (TR / EN)',
      'keys.shot': 'Save a screenshot',
      'keys.help': 'Help overlay',
      'keys.quit': 'Release the cursor, press again to quit',
      'play.note': 'Mouse look is optional. The cursor is never locked; M toggles capture and Esc always releases it.',

      'agents.title': 'Three deterministic agents',
      'agents.intro': 'The agents are task state machines running fixed rules. They do not use a language model to make decisions. States: Idle → Navigating → Working → Completed or Failed.',
      'agents.patrol.title': 'Patrol',
      'agents.patrol.body': 'Walks a loop of sidewalk corners and repeats the round.',
      'agents.deliver.title': 'Deliver',
      'agents.deliver.body': 'Picks up a package at the depot, carries it to a storefront, and returns. The carried package is drawn above the agent.',
      'agents.survey.title': 'Survey',
      'agents.survey.body': 'Travels to a point, works there briefly, and writes the result to the panel.',
      'agents.note': 'Pathfinding never crosses a building. When a target is unreachable the agent fails openly with "no route" instead of faking success, and then takes a new task.',

      'download.title': 'Download',
      'download.intro': 'You do not need to compile the source. Packages are published on GitHub Releases.',
      'download.win.file': 'TinyTown-v0.1.0-windows-x64.zip',
      'download.win.body': 'Extract the ZIP and double-click tinytown.exe. The SDL2 runtime is included in the package.',
      'download.linux.file': 'TinyTown-v0.1.0-linux-x64.tar.gz',
      'download.linux.body': 'Extract the archive and run ./tinytown. SDL2 and OpenGL are system dependencies and are not bundled.',
      'download.cta': 'GitHub Releases page',
      'download.source': 'Source and tests',
      'download.note': 'Downloading does not update an installed copy on your computer. There is no auto-updater in this release.',

      'honest.title': 'What this release does not do',
      'honest.intro': 'We state this plainly to avoid false expectations. The following are not part of v0.1.0:',
      'honest.li1': 'An LLM, learning AI, TTS, or AI image generation.',
      'honest.li2': 'Audio, WAV export, MP4, or animation frame sequences.',
      'honest.li3': 'A browser / WebAssembly build.',
      'honest.li4': 'An auto-updater.',
      'honest.browser': 'The application is a native SDL2 window using compatibility-profile immediate-mode OpenGL. It does not run in a browser as it stands; a browser target is a separate milestone and needs a real WebGL/GLES renderer plus an Emscripten build.',

      'status.title': 'Status',
      'status.caption': 'Platform verification status',
      'status.th.platform': 'Platform',
      'status.th.build': 'Build',
      'status.th.tests': 'Tests',
      'status.th.gui': 'Interface launched',
      'status.yes': 'Yes',
      'status.no': 'No',
      'status.tests77': '77 / 77',
      'status.linuxGui': 'Yes, with Xvfb + Mesa',
      'status.winGui': 'No, runner has no display',
      'status.browser': 'Browser',
      'status.note': 'On Windows the application builds and the tests pass, but the interactive window and its OpenGL rendering were not exercised on the CI runner. That is not a substitute for a real desktop session.',

      'roadmap.title': 'Roadmap',
      'roadmap.intro': 'The next steps are separate, measurable tasks. None of them is part of v0.1.0, and none is shown as complete.',
      'roadmap.v010.title': 'v0.1.0 — Living world and publishable base',
      'roadmap.v010.body': 'Done: the procedural town, the walkable camera, three deterministic agents, the Turkish/English interface, tests, packages, the release, and this site.',
      'roadmap.v020.title': 'v0.2 — Capturable and shareable media output',
      'roadmap.v020.body': 'Planned, not started: deterministic animation frame sequences, code-synthesized ambient audio with volume control, WAV export, and optional MP4 via FFmpeg.',
      'roadmap.v030.title': 'v0.3 — Wider reach and browser evaluation',
      'roadmap.v030.body': 'Planned, not started: re-verify native Windows/Linux packages, assess a WebGL/GLES-compatible renderer, and build a working browser prototype. No browser build will be claimed until a real one is published.',

      'footer.license': 'TinyTown is released under the MIT License. SDL2 (zlib licence) is the only external dependency.',
      'footer.repo': 'Repository',
      'footer.notices': 'Third-party notices',
      'footer.roadmap': 'Roadmap'
    }
  };

  var STORAGE_KEY = 'tinytown.lang';
  var DEFAULT_LANG = 'tr';

  function readStoredLanguage() {
    try {
      var value = window.localStorage.getItem(STORAGE_KEY);
      return STRINGS[value] ? value : null;
    } catch (error) {
      // Storage can be blocked. Fall back to the default without failing.
      return null;
    }
  }

  function storeLanguage(lang) {
    try {
      window.localStorage.setItem(STORAGE_KEY, lang);
    } catch (error) {
      // Not being able to remember the choice is acceptable.
    }
  }

  function applyLanguage(lang) {
    var table = STRINGS[lang] || STRINGS[DEFAULT_LANG];
    var missing = [];

    document.querySelectorAll('[data-i18n]').forEach(function (node) {
      var key = node.getAttribute('data-i18n');
      if (Object.prototype.hasOwnProperty.call(table, key)) {
        node.textContent = table[key];
      } else {
        missing.push(key);
      }
    });

    document.querySelectorAll('[data-i18n-alt]').forEach(function (node) {
      var key = node.getAttribute('data-i18n-alt');
      if (Object.prototype.hasOwnProperty.call(table, key)) {
        node.setAttribute('alt', table[key]);
      } else {
        missing.push(key);
      }
    });

    document.documentElement.lang = lang;

    document.querySelectorAll('.lang-btn').forEach(function (button) {
      var isActive = button.getAttribute('data-lang') === lang;
      button.classList.toggle('is-active', isActive);
      button.setAttribute('aria-pressed', isActive ? 'true' : 'false');
    });

    if (missing.length > 0) {
      // Surface the gap in the console instead of silently leaving stale text.
      console.warn('TinyTown page: missing translations for', missing);
    }
  }

  function init() {
    var initial = readStoredLanguage() || DEFAULT_LANG;
    applyLanguage(initial);

    document.querySelectorAll('.lang-btn').forEach(function (button) {
      button.addEventListener('click', function () {
        var lang = button.getAttribute('data-lang');
        if (!STRINGS[lang]) return;
        storeLanguage(lang);
        applyLanguage(lang);
      });
    });
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }
})();
