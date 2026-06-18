#!/usr/bin/env bash
#
# setup.sh — environment provisioning for smart-home-hub
#
# Scope: prepares the environment so `cmake && make` can be run.
# Does NOT build the project itself, and does NOT touch OS-level
# concerns (Pi OS flashing, Hailo8 PCIe driver/firmware) — those are
# assumed to already be in place.
#
# Idempotent: safe to re-run; each step skips work that's already done.
# Fails fast: halts on the first error with an explanatory message.

set -uo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_DIR="$PROJECT_ROOT/config"
MODELS_DIR="$PROJECT_ROOT/models"
SETTINGS_FILE="$CONFIG_DIR/settings.json"
HAILO_APPS_DIR="$PROJECT_ROOT/hailo-apps"

HAILO_APPS_REPO_URL="https://github.com/hailo-ai/hailo-apps.git"

# TODO(confirm): this is a stand-in idempotency marker for "llama.cpp is
# installed system-wide". Replace with whatever file/command actually
# reflects how it was installed on your machines (e.g. a specific header,
# a pkg-config entry, or `ldconfig -p | grep libllama`).
LLAMA_INSTALL_MARKER="/usr/local/include/llama.h"

log() { echo "[setup] $*"; }
die() { echo "[setup] ERROR: $*" >&2; exit 1; }

# ---------------------------------------------------------------------------
# 1. Base system packages
# ---------------------------------------------------------------------------
install_system_packages() {
    log "Checking base system packages..."
    local packages=(build-essential cmake git wget curl python3-pip jq libssl-dev libboost-all-dev libcurl4-openssl-dev ffmpeg libportaudio2 ninja-build)
    local missing=()

    for pkg in "${packages[@]}"; do
        dpkg -s "$pkg" >/dev/null 2>&1 || missing+=("$pkg")
    done

    if [ "${#missing[@]}" -gt 0 ]; then
        log "Installing missing packages: ${missing[*]}"
        sudo apt-get update -y || die "apt-get update failed"
        sudo apt-get install -y "${missing[@]}" || die "failed to install: ${missing[*]}"
    else
        log "All base system packages already present."
    fi
}

# ---------------------------------------------------------------------------
# 2. Config files (tadiran_config.json, authorized_users.json)
# ---------------------------------------------------------------------------
ensure_config_file() {
    local target="$1"
    local example="$2"

    if [ -f "$target" ]; then
        log "$(basename "$target") already exists, skipping."
    elif [ -f "$example" ]; then
        cp "$example" "$target"
        log "Created $(basename "$target") from example template."
        log ">>> Edit $target and fill in your actual values before running the app."
    else
        die "$(basename "$example") not found; cannot create $(basename "$target")"
    fi
}

setup_config_files() {
    log "Checking local config files..."
    ensure_config_file "$CONFIG_DIR/tadiran_config.json" "$CONFIG_DIR/tadiran_config.example.json"
    ensure_config_file "$CONFIG_DIR/authorized_users.json" "$CONFIG_DIR/authorized_users.example.json"
}

# ---------------------------------------------------------------------------
# 3. Resolve the active model from settings.json
# ---------------------------------------------------------------------------
# Sets globals: ENCODER, NEEDS_MODEL, ACTIVE_MODEL, MODEL_PATH, MODEL_URL,
#               MODEL_SIZE_MB, MODEL_MIN_RAM_MB
resolve_active_model() {
    [ -f "$SETTINGS_FILE" ] || die "settings.json not found at $SETTINGS_FILE"

    ENCODER=$(jq -r '.encoder' "$SETTINGS_FILE")
    ACTIVE_MODEL=$(jq -r '.active_model' "$SETTINGS_FILE")

    if [ "$ENCODER" != "llama" ]; then
        log "Active encoder is '$ENCODER' — no model download or llama.cpp build needed."
        NEEDS_MODEL=0
        return
    fi

    NEEDS_MODEL=1

    local entry
    entry=$(jq -c --arg p "$ACTIVE_MODEL" \
        '.available_models[$p]' "$SETTINGS_FILE")

    if [ "$entry" == "null" ]; then
        die "Active model '$ACTIVE_MODEL' not found in available_models."
    fi

    MODEL_PATH=$(echo "$entry" | jq -r '.path')
    MODEL_URL=$(echo "$entry" | jq -r '.url')
    MODEL_SIZE_MB=$(echo "$entry" | jq -r '.expected_size_mb')
    MODEL_MIN_RAM_MB=$(echo "$entry" | jq -r '.min_ram_mb')
}

# ---------------------------------------------------------------------------
# 4. RAM check (model must fit in RAM to run, regardless of download status)
# ---------------------------------------------------------------------------
check_ram() {
    local total_ram_mb
    total_ram_mb=$(free -m | awk '/^Mem:/{print $2}')

    if [ "$total_ram_mb" -lt "$MODEL_MIN_RAM_MB" ]; then
        die "Insufficient RAM for the selected model: have ${total_ram_mb}MB, need ${MODEL_MIN_RAM_MB}MB. Try selecting a smaller model in settings.json."
    fi

    log "RAM check passed (${total_ram_mb}MB total)."
}

# ---------------------------------------------------------------------------
# 5. Model existence check (sets MODEL_EXISTS; drives disk check + download)
# ---------------------------------------------------------------------------
check_model_exists() {
    local full_path="$PROJECT_ROOT/$MODEL_PATH"

    if [ -f "$full_path" ]; then
        MODEL_EXISTS=1
        log "Model already present at $MODEL_PATH."
    else
        MODEL_EXISTS=0
    fi
}

# ---------------------------------------------------------------------------
# 6. Disk space check (only relevant if the model still needs downloading)
# ---------------------------------------------------------------------------
check_disk_space() {
    mkdir -p "$MODELS_DIR"
    local avail_mb
    avail_mb=$(df --output=avail -BM "$MODELS_DIR" | tail -1 | tr -dc '0-9')

    if [ "$avail_mb" -lt "$MODEL_SIZE_MB" ]; then
        die "Insufficient disk space for model download: have ${avail_mb}MB free, need ${MODEL_SIZE_MB}MB."
    fi

    log "Disk space check passed (${avail_mb}MB free)."
}

# ---------------------------------------------------------------------------
# 7. llama.cpp — system-wide build/install
# ---------------------------------------------------------------------------
setup_llama_cpp() {
    if [ -f "$LLAMA_INSTALL_MARKER" ]; then
        log "llama.cpp already installed system-wide, skipping."
        return
    fi

    log "Building and installing llama.cpp system-wide..."
    local tmp_dir
    tmp_dir=$(mktemp -d)

    git clone https://github.com/ggerganov/llama.cpp.git "$tmp_dir/llama.cpp" \
        || die "failed to clone llama.cpp"

    (
        cd "$tmp_dir/llama.cpp" || exit 1
        cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLAMA_BUILD_TESTS=OFF -DLLAMA_BUILD_EXAMPLES=OFF || exit 1
        cmake --build build -j"$(nproc)" || exit 1
        sudo cmake --install build || exit 1
        sudo ldconfig || exit 1
    ) || die "llama.cpp build/install failed"

    rm -rf "$tmp_dir"
}

# ---------------------------------------------------------------------------
# 8. Model download (only if MODEL_EXISTS is false)
# ---------------------------------------------------------------------------
download_model_if_missing() {
    if [ "$MODEL_EXISTS" -eq 1 ]; then
        log "Model already present at $MODEL_PATH, skipping download."
        return
    fi

    local full_path="$PROJECT_ROOT/$MODEL_PATH"
    log "Downloading model from $MODEL_URL ..."
    mkdir -p "$(dirname "$full_path")"
    wget -O "$full_path" "$MODEL_URL" || die "model download failed"
}

# ---------------------------------------------------------------------------
# 9. tinytuya (Python Tuya bridge dependency)
# ---------------------------------------------------------------------------
install_tinytuya() {
    if python3 -c "import tinytuya" >/dev/null 2>&1; then
        log "tinytuya already installed, skipping."
        return
    fi

    log "Installing tinytuya..."
    pip3 install tinytuya --break-system-packages || die "failed to install tinytuya"
}

# ---------------------------------------------------------------------------
# 10. hailo-apps (speech-to-text dependency, independent of encoder choice)
# ---------------------------------------------------------------------------
setup_hailo_apps() {
    if [ -d "$HAILO_APPS_DIR" ]; then
        log "hailo-apps already cloned, skipping clone."
    else
        log "Cloning hailo-apps..."
        git clone "$HAILO_APPS_REPO_URL" "$HAILO_APPS_DIR" || die "failed to clone hailo-apps"
    fi

    if python3 -c "import hailo_apps" >/dev/null 2>&1; then
        log "hailo-apps already installed (importable), skipping pip install."
        return
    fi

    log "Installing hailo-apps (speech-rec extras)..."
    (
        cd "$HAILO_APPS_DIR" || exit 1
        pip3 install -e ".[speech-rec]" --break-system-packages --ignore-installed || exit 1
    ) || die "failed to pip install hailo-apps"
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
main() {
    log "Starting smart-home-hub environment setup..."

    install_system_packages
    setup_config_files
    resolve_active_model

    if [ "$NEEDS_MODEL" -eq 1 ]; then
        check_ram
        check_model_exists
        if [ "$MODEL_EXISTS" -eq 0 ]; then
            check_disk_space
        fi
        setup_llama_cpp
        download_model_if_missing
    fi

    install_tinytuya
    setup_hailo_apps

    log "Setup complete. Project is ready to build: run cmake && make from the project root."
}

main "$@"