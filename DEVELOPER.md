# Developer guide – local compilation test

This document explains how to verify that the component compiles without errors,
without needing the physical hardware and without touching your system Python.

---

## Option A – Python virtualenv (recommended)

### 1. Create an isolated environment

```bash
cd /path/to/esphome-gdey075t7   # repo root
python3 -m venv .venv
source .venv/bin/activate        # on Windows: .venv\Scripts\activate
```

### 2. Install ESPHome

```bash
pip install esphome
```

Verify the install:

```bash
esphome version
```

### 3. Create a secrets file

The example YAML uses `!secret` for Wi-Fi credentials. ESPHome looks for
`secrets.yaml` in the same directory as the config file.

```bash
cat > components/gdey075t7_gxepd2/example/secrets.yaml <<'EOF'
wifi_ssid: "TestNetwork"
wifi_password: "testpassword"
EOF
```

(The credentials don't need to be real — the compile step doesn't connect to
anything.)

### 4. Compile

```bash
esphome compile components/gdey075t7_gxepd2/example/xiao_gdey075t7.yaml
```

ESPHome will:
1. Validate the YAML and run the Python config layer (`display.py`)
2. Download PlatformIO, the ESP32 Arduino toolchain, and the GxEPD2 library
   (first run takes several minutes)
3. Compile all C++ sources

A successful run ends with:

```
========================= [SUCCESS] Took ...s =========================
```

Any C++ error or Python config error appears here before flashing is needed.

### 5. Deactivate when done

```bash
deactivate
```

---

## Option B – Docker (no Python setup at all)

Use the official ESPHome container. Nothing is installed on the host.

```bash
# from the repo root
docker run --rm \
  -v "$(pwd)":/config \
  -w /config \
  ghcr.io/esphome/esphome:latest \
  compile components/gdey075t7_gxepd2/example/xiao_gdey075t7.yaml
```

Create the secrets file first (same as step 3 above, path relative to repo root):

```bash
cat > components/gdey075t7_gxepd2/example/secrets.yaml <<'EOF'
wifi_ssid: "TestNetwork"
wifi_password: "testpassword"
EOF
```

---

## What the compile step actually checks

| Check | How it's triggered |
|---|---|
| Python schema validation | ESPHome parses the YAML and runs `display.py` |
| C++ syntax + includes | `gcc-xtensa` compiles every `.cpp` file |
| Library availability | PlatformIO fetches `zinggjm/GxEPD2` from its registry |
| `InternalGPIOPin::get_pin()` linkage | Linker resolves all symbols |

If only the config layer needs checking (faster, skips C++ compilation):

```bash
esphome config components/gdey075t7_gxepd2/example/xiao_gdey075t7.yaml
```

---

## Cleaning up build artefacts

ESPHome writes build artefacts to a hidden `.esphome/` folder next to the config
file. To do a clean rebuild:

```bash
rm -rf components/gdey075t7_gxepd2/example/.esphome
```

---

## Pinning the ESPHome version

To test against a specific ESPHome release:

```bash
pip install "esphome==2024.12.0"
```

Check the current release at https://github.com/esphome/esphome/releases.
