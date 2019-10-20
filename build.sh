#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
CYAN='\033[0;36m'
NC='\033[0m'



num_args=$#
should_flash=0
if [ ${num_args} -gt 0 ]; then
  if [ "$1" == "--flash" -o "$1" == "-f" ]; then
    should_flash=1
  else
    echo -e "${RED}ERROR: Invalid argument '$1'.${NC}"
    exit 1
  fi
fi

# First use the Arduino compile to generate a .hex file by "verifying" the
# .ino file and specifying a custom build directory
BOARD="MightyCore:avr:16:pinout=standard,clock=16MHz_external"
ACTION="--verify"
ARDUINO="arduino"
INO="led_ray_clock.ino"
BUILD_PATH="/tmp/arduino_builds"

echo -e "${CYAN}++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++${NC}"
echo -e "${CYAN}COMPILE: Compiling with the Arduino compiler...${NC}"
JAVA_TOOL_OPTIONS='-Djava.awt.headless=true' "${ARDUINO}" "${ACTION}" "${INO}" --board "${BOARD}" --pref build.path=${BUILD_PATH}
ret=$?
if [ ${ret} -ne 0 ]; then
  echo -e "${RED}ERROR: Arduino returned error code '${ret}'.  Exiting${NC}"
  exit ${ret}
else
  echo -e "${GREEN}SUCCESS: Compilation completed without errors.${NC}"
  echo
fi

if [ ${should_flash} -ne 1 ]; then
  echo -e "${CYAN}(If you want to flash the MCU, rerun with the '-f' or '--flash' option.)${NC}"
  exit 0
fi


# Now that's it's compiled, we start the process of actually flashing the
# program onto our MCU.  In the Arduino build directory there is a .hex file that
# includes our program and the arduino bootloader.  After confirming the MCU is
# connected/setu up correctly we use avrdude to flash that whole thing onto the Atmega16.
DEVICE="atmega16"
PROGRAMMER="usbtiny"
AVRDUDE="avrdude -c ${PROGRAMMER} -p ${DEVICE}"

# Confirm that the expected programmer and device are connected
echo -e "${CYAN}++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++${NC}"
echo -e "${CYAN}CONNECTION TEST: Plumbing test for programmer & mcu...${NC}"
${AVRDUDE} -qq
ret=$?
if [ ${ret} -ne 0 ]; then
  echo -e "${RED}ERROR: Failed to communicate with programmer/mcu! (avrdude returned ${ret})${NC}"
  exit ${ret}
else
  echo -e "${GREEN}SUCCESS: Programmer and mcu responded as expected.${NC}"
fi
echo

# Update Fuses, if need be
LFUSE="0xFF"
HFUSE="0x89"
VERIFY_FUSES="${AVRDUDE} -U lfuse:v:${LFUSE}:m -U hfuse:v:${HFUSE}:m -qq"
WRITE_FUSES="${AVRDUDE} -U lfuse:w:${LFUSE}:m -U hfuse:w:${HFUSE}:m -qq"
HEX_FILE="${BUILD_PATH}/${INO}.hex"

echo -e "${CYAN}++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++${NC}"
echo -e "${CYAN}FUSES: Making sure the fuses are L:${LFUSE} and H:${HFUSE}...${NC}"
${VERIFY_FUSES}
if [ $? -ne 0 ]; then
  echo -e "${YELLOW}Incorrect fuse values found, attempting to update them now...${NC}"
  ${WRITE_FUSES}
  echo -e "${YELLOW}Verifying the newly written values...${NC}"
  ${VERIFY_FUSES}
  ret=$?
  if [ ${ret} -ne 0 ]; then
    echo -e "${RED}ERROR: Incorrect fuse values still found! (${ret}) Exiting...${NC}"
    exit ${ret}
  else
    echo -e "${GREEN}SUCCESS: Fuse values sucessfully updated.${NC}"
  fi
else
  echo -e "${GREEN}SUCCESS: Fuse values were already up-to-date, no update needed.${NC}"
fi
echo

# Perform the actual flashing operation
echo -e "${CYAN}++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++${NC}"
echo -e "${CYAN}FLASHING: Flashing the compiled binary onto the MCU...${NC}"
${AVRDUDE} -U flash:w:${HEX_FILE}:i
ret=$?
if [ ${ret} -ne 0 ]; then
  echo -e "${RED}ERROR: avrdude returned ${ret} while attempting to flash.${NC}"
  exit ${ret}
else
  echo -e "${GREEN}SUCCESS: Program flashed successfully!${NC}"
fi

exit 0
