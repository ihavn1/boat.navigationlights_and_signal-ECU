"""
PlatformIO script to read OTA password from secrets.h and inject into espota uploads
This keeps passwords out of platformio.ini (which may be committed to git)
"""
Import("env")
import re

def extract_ota_password():
    """Read OTA_PASSWORD from secrets.h"""
    try:
        with open("src/secrets.h", "r") as f:
            content = f.read()
            match = re.search(r'#define\s+OTA_PASSWORD\s+"([^"]+)"', content)
            if match:
                password = match.group(1)
                print(f"✓ OTA password loaded from secrets.h")
                return password
    except Exception as e:
        print(f"⚠ Warning: Could not read OTA_PASSWORD from secrets.h: {e}")
    return None

def on_upload(source, target, env):
    """Called before upload - inject OTA password if doing network upload"""
    upload_port = env.get("UPLOAD_PORT", "")
    
    # Check if it's a network upload
    if upload_port and not upload_port.startswith("COM") and not upload_port.startswith("/dev/"):
        password = extract_ota_password()
        if password:
            # Find and update the auth flag in UPLOADERFLAGS
            flags = env.get("UPLOADERFLAGS", [])
            # Remove any existing --auth flag
            flags = [f for f in flags if not f.startswith("--auth=")]
            # Add our password
            flags.append(f"--auth={password}")
            env.Replace(UPLOADERFLAGS=flags)
            print(f"✓ OTA authentication configured for {upload_port}")
        else:
            print("⚠ Warning: Network upload without OTA_PASSWORD - authentication will fail")

# Register the callback for both firmware and filesystem uploads
env.AddPreAction("upload", on_upload)
env.AddPreAction("uploadfs", on_upload)

