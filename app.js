(function () {
  'use strict';

  var launched = {};
  var currentFilter = 'all';
  var currentSearch = '';

  // ── iOS 13 launch technique ──────────────────────────────────────────────
  // Uses hidden iframe + timed window.location for best iOS 13 compat.
  // Both run inside the user-gesture propagation window.
  function iosLaunch(scheme, chipEl) {
    if (!scheme) return;
    if (!scheme.includes('://') && !scheme.includes(':')) scheme += '://';

    // iframe approach (silent, no address bar flicker)
    var iframe = document.createElement('iframe');
    iframe.style.cssText = 'display:none;width:0;height:0;position:absolute;';
    document.body.appendChild(iframe);
    try { iframe.src = scheme; } catch (e) {}

    var t = Date.now();
    setTimeout(function () {
      if (Date.now() - t < 1000) {
        try { window.location.href = scheme; } catch (e) {}
      }
      if (iframe.parentNode) iframe.parentNode.removeChild(iframe);
    }, 25);

    // UI feedback
    if (chipEl) {
      chipEl.classList.add('launched');
      chipEl.querySelector('.dot').title = 'launched';
    }
    launched[scheme] = true;
    showToast('→ ' + scheme);
    logLaunch(scheme);
  }

  // ── toast ────────────────────────────────────────────────────────────────
  var toastTimer;
  function showToast(msg) {
    var t = document.getElementById('toast');
    t.textContent = msg;
    t.classList.remove('hidden');
    clearTimeout(toastTimer);
    toastTimer = setTimeout(function () { t.classList.add('hidden'); }, 2000);
  }

  // ── log ──────────────────────────────────────────────────────────────────
  var log = [];
  function logLaunch(scheme) {
    log.unshift({ scheme: scheme, time: new Date().toLocaleTimeString() });
    if (log.length > 50) log.pop();
  }

  // ── custom launch ────────────────────────────────────────────────────────
  window.launchCustom = function () {
    var v = document.getElementById('customScheme').value.trim();
    if (!v) { showToast('Enter a scheme first'); return; }
    iosLaunch(v, null);
  };

  document.getElementById('customScheme').addEventListener('keydown', function (e) {
    if (e.key === 'Enter') window.launchCustom();
  });

  // ── build category list ──────────────────────────────────────────────────
  var cats = [];
  var catSet = {};
  SCHEMES.forEach(function (s) {
    if (!catSet[s.cat]) { catSet[s.cat] = true; cats.push(s.cat); }
  });
  cats.sort();

  var sel = document.getElementById('categoryFilter');
  cats.forEach(function (c) {
    var o = document.createElement('option');
    o.value = c; o.textContent = c;
    sel.appendChild(o);
  });

  sel.addEventListener('change', function () {
    currentFilter = sel.value;
    render();
  });

  document.getElementById('search').addEventListener('input', function (e) {
    currentSearch = e.target.value.trim().toLowerCase();
    render();
  });

  // ── render ────────────────────────────────────────────────────────────────
  function render() {
    var grid = document.getElementById('grid');
    var stats = document.getElementById('stats');
    grid.innerHTML = '';

    var q = currentSearch;
    var filtered = SCHEMES.filter(function (s) {
      if (currentFilter !== 'all' && s.cat !== currentFilter) return false;
      if (q && s.scheme.toLowerCase().indexOf(q) === -1 && s.cat.toLowerCase().indexOf(q) === -1) return false;
      return true;
    });

    // dedupe by scheme
    var seen = {};
    filtered = filtered.filter(function (s) {
      if (seen[s.scheme]) return false;
      seen[s.scheme] = true;
      return true;
    });

    stats.textContent = filtered.length + ' of ' + SCHEMES.length + ' schemes shown';

    if (!filtered.length) {
      grid.innerHTML = '<div class="no-results">No schemes match your search.</div>';
      return;
    }

    // group by category
    var groups = {};
    filtered.forEach(function (s) {
      if (!groups[s.cat]) groups[s.cat] = [];
      groups[s.cat].push(s);
    });

    Object.keys(groups).sort().forEach(function (cat) {
      var block = document.createElement('div');
      block.className = 'category-block';

      var title = document.createElement('div');
      title.className = 'cat-title';
      title.textContent = cat + ' (' + groups[cat].length + ')';
      block.appendChild(title);

      var wrap = document.createElement('div');
      wrap.className = 'chip-wrap';

      groups[cat].forEach(function (s) {
        var chip = document.createElement('div');
        chip.className = 'chip';
        if (s.hidden) chip.classList.add('hidden-app');
        if (s.deep)   chip.classList.add('deep-link');
        if (launched[s.scheme]) chip.classList.add('launched');

        var dot = document.createElement('span');
        dot.className = 'dot';
        var label = document.createElement('span');
        label.textContent = s.scheme;

        chip.appendChild(dot);
        chip.appendChild(label);

        chip.addEventListener('click', function () {
          iosLaunch(s.scheme, chip);
        });

        wrap.appendChild(chip);
      });

      block.appendChild(wrap);
      grid.appendChild(block);
    });
  }

  render();
})();
