# LWCloneU2 (Due) – PlatformIO-Zusatzskript: die gemeinsamen C-Module aus ../firmware mitbauen und die
# Ersatz-Header (compat/) für avr-libc-Includes bereitstellen.
# PlatformIO extra script: build the shared C modules from ../firmware and provide the stand-in headers
# (compat/) for avr-libc includes.
import os

Import("env")

project_dir = env.subst("$PROJECT_DIR")
src_dir = env.subst("$PROJECT_SRC_DIR")
firmware_dir = os.path.abspath(os.path.join(project_dir, "..", "firmware"))

env.Append(CPPPATH=[
    os.path.join(src_dir, "compat"),
    src_dir,
    firmware_dir,
])

# Nur diese Module sind hardwareunabhängig; pins.c, ws2812.c, clock.c, main_*.c haben Due-Fassungen in src/.
# Only these modules are hardware independent; pins.c, ws2812.c, clock.c, main_*.c have Due versions in src/.
env.BuildSources(
    os.path.join("$BUILD_DIR", "lwc_shared"),
    firmware_dir,
    src_filter=[
        "-<*>",
        "+<proto.c>",
        "+<config.c>",
        "+<outputs.c>",
        "+<panel.c>",
        "+<strip.c>",
        "+<strip_serial.c>",
    ],
)
