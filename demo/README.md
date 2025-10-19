Windows 1 themed interactive demo
=================================

This small demo is a recruiter-friendly interactive Python application that mimics the look-and-feel of Windows 1: a tiled desktop, a Program Manager window, and a Start button that opens a simple menu.

Features added:
- Click and drag windows (bring-to-front behavior)
- Start button and simple menu
- Lightweight, runs with pygame

How to run:

1. Install dependencies from the project root:

```pwsh
python -m pip install -r requirements.txt
```

2. Run the demo:

```pwsh
python -m demo.main
```

Optional: Launch using SCons if you want scons to run it via the `demo` phony target (added to SConstruct):

```pwsh
scons demo
```
