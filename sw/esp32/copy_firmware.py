Import("env")
import configparser
import shutil

# script that copies the firmware into the root directory for easy copying

def copy_firmware(source, target, env):
    config = configparser.ConfigParser()
    config.read("platformio.ini")
    destination = config.get("custom_buildargs", "firmware_destination")
    firmware = source[0].get_abspath()
    shutil.copy(firmware, destination);

env.AddPostAction("buildprog", copy_firmware)
