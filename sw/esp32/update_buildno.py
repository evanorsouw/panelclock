import re
import os
import configparser

# script that updates the version number in the version.h files
# and that generates a manifest of all files required for production

def update_build_number(file_path, data_path, manifest_file_path):
    # Regular expression to match the VERSION line
    version_pattern = re.compile(r'(#define\s+VERSION\s+"(\d+)\.(\d+)\.(\d+)")')

    with open(file_path, 'r') as file:
        lines = file.readlines()

    updated_lines = []
    for line in lines:
        match = version_pattern.search(line)
        if match:
            major, minor, build = match.groups()[1:]  # Extract major, minor, build numbers
            new_build = int(build) + 1  # Increment the build number
            # Replace the matched line with the updated version
            new_line = f'#define VERSION "{major}.{minor}.{new_build}"\n'
            updated_lines.append(new_line)
        else:
            updated_lines.append(line)

    # Write the updated lines back to the file
    with open(file_path, 'w') as file:
        file.writelines(updated_lines)

    print(f"Updated build number in {file_path} from {build} to {new_build}")

    manifest_lines = []
    manifest_lines.append(f'firmware={major}.{minor}.{new_build}\n')

    for entry in os.listdir(data_path):
        entry_path = os.path.join(data_path, entry)  # Full path to the file
        if os.path.isfile(entry_path) and entry != manifest_file_path:  # Check if it's a file
            manifest_lines.append(entry + "\n")
    print(f"Writing manifest")
    with open(os.path.join(data_path, manifest_file_path), "w") as file:
        for line in manifest_lines:
            file.write(line)


config = configparser.ConfigParser()
config.read("platformio.ini")
version_file = config.get("custom_buildargs", "version_file", fallback="src/version.cpp")
data_path = config.get("custom_buildargs", "data_path", fallback="data")
manifest_file = config.get("custom_buildargs", "manifest_file", fallback="manifest.txt")

update_build_number(version_file, data_path, manifest_file)
