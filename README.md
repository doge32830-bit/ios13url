# iOS URL Scheme Launcher

A static website for launching iOS apps via URL schemes directly from Safari on your iPhone. Specifically tuned for **iOS 13** compatibility.

## Live demo

Host on GitHub Pages and open on your iPhone in Safari.

## Features

- 300+ URL schemes across 20+ categories
- Hidden/private Apple system app schemes (marked with red dot)
- Deep-link schemes with paths/parameters (marked with blue dot)
- Search and filter by category
- Custom scheme input
- iOS 13–optimised launch technique (hidden iframe + timed `window.location`)

## How iOS 13 launching works

iOS 13 requires URL scheme navigation to happen within the **user-gesture propagation window** (~1 second after a tap). This site uses two parallel techniques:

1. **Hidden iframe** — sets `iframe.src = scheme` silently, no address bar flicker
2. **Timed window.location** — `window.location.href = scheme` inside `setTimeout(fn, 25)` to avoid same-tick sandboxing quirks while staying within the gesture window

Both fire from the same tap event, maximising the chance at least one succeeds.

## Hosting on GitHub Pages

1. Fork or clone this repo
2. Go to **Settings → Pages**
3. Set source to **main branch / root**
4. Visit `https://yourusername.github.io/ios-scheme-launcher/` on your iPhone

## URL scheme categories

| Category | Description |
|---|---|
| Apple Hidden | Private/internal Apple system app schemes (may require specific iOS versions) |
| Phone & FaceTime | `tel://`, `facetime://`, etc. |
| Settings | `prefs:root=*` deep links into Settings |
| Shortcuts & Automation | `shortcuts://`, `workflow://`, launch apps |
| Social Media | Twitter, Instagram, WhatsApp, etc. |
| Productivity | Notes apps, task managers, text editors |
| ...and more | Music, Video, Maps, Cloud, Finance, etc. |

## Legend

- 🔴 Red dot = hidden/private Apple system app (may not work on all devices)
- 🔵 Blue dot = deep link with path or parameters
- 🟢 Green dot = launched successfully this session

## Notes

- **Safari only** — third-party browsers restrict custom URL scheme navigation even with user gestures
- **App must be installed** — iOS silently fails if the target app isn't installed (no error in Safari)
- **iOS 14+ behaviour differs** — some private schemes that work on iOS 13 were restricted in later versions
- Schemes marked as hidden/private were extracted from iOS system files and may not work without specific entitlements

## Sources

- [Justin Meyers — Complete List of iOS URL Schemes (Medium)](https://medium.com/@contact.jmeyers)
- [phynet/iOS-URL-Schemes (GitHub)](https://github.com/phynet/iOS-URL-Schemes)
- [ChronSyn/BIG iOS URL SCHEME LIST (Gist)](https://gist.github.com/ChronSyn/dffdffc037bb62731f35fbbda7e8e440)
- [bhagyas/app-urls (GitHub)](https://github.com/bhagyas/app-urls)
- [Apple Developer Documentation](https://developer.apple.com/library/archive/featuredarticles/iPhoneURLScheme_Reference/)
- [kairin/iphoneprefsurls (GitHub)](https://github.com/kairin/iphoneprefsurls)

## File structure

```
ios-scheme-launcher/
├── index.html      # Main page
├── style.css       # Styles (dark mode, iOS-native feel)
├── schemes.js      # All URL schemes data
├── app.js          # Launch logic + UI rendering
└── README.md       # This file
```

## Contributing

Found a scheme not in the list? Open a PR and add it to `schemes.js` with the correct category and flags.
