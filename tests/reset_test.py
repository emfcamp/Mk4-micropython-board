#
# Plug the board in (not the reset button) and then run this script three times,
# each time following the prompts
#

from machine import reset, reset_cause, PWRON_RESET, HARD_RESET, SOFT_RESET
from time import sleep

print("PWR: {}, HARD: {}, SOFT: {}".format(PWRON_RESET, HARD_RESET, SOFT_RESET))
print("CC32xx does not have HARD_RESET")

cause = reset_cause()
print("cause: {}".format(cause))
print("cause: {} (again)".format(reset_cause()))

if cause == PWRON_RESET:
    print("Executing soft reset now - run this script after reboot")
    reset()
elif cause == HARD_RESET:
    print("Done with reset tests")
elif cause == SOFT_RESET:
    print("Press the board reset button - run this script after reboot")
    sleep(100)
else:
    print("Unknown reset cause")
