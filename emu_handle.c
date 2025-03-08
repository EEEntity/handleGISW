#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <errno.h>

#define msleep(ms) usleep((ms) * 1000)

// Function to set up an absolute axis channel on the uinput device.
static void setup_abs(int fd, unsigned chan, int min, int max) {
    if (ioctl(fd, UI_SET_ABSBIT, chan) < 0)
        perror("UI_SET_ABSBIT");
    
    struct uinput_abs_setup s = {
        .code = chan,
        .absinfo = { .minimum = min, .maximum = max }
    };
    
    if (ioctl(fd, UI_ABS_SETUP, &s) < 0)
        perror("UI_ABS_SETUP");
}

// Function to simulate a button click: a press then a release.
static void simulate_click(int fd, int button) {
    struct input_event ev;
    
    // Button press
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_KEY;
    ev.code = button;
    ev.value = 1; // Press
    if (write(fd, &ev, sizeof(ev)) < 0)
        perror("write press");
    
    // Sync event after press
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    if (write(fd, &ev, sizeof(ev)) < 0)
        perror("write SYN after press");
    
    // Short delay between press and release (optional within the click)
    msleep(50);
    
    // Button release
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_KEY;
    ev.code = button;
    ev.value = 0; // Release
    if (write(fd, &ev, sizeof(ev)) < 0)
        perror("write release");
    
    // Sync event after release
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    if (write(fd, &ev, sizeof(ev)) < 0)
        perror("write SYN after release");
}

int main(void) {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("open /dev/uinput");
        return 1;
    }

    // Enable key events and configure supported buttons.
    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0) perror("UI_SET_EVBIT EV_KEY");

    // Primary buttons
    if (ioctl(fd, UI_SET_KEYBIT, BTN_A) < 0) perror("UI_SET_KEYBIT BTN_A");
    if (ioctl(fd, UI_SET_KEYBIT, BTN_B) < 0) perror("UI_SET_KEYBIT BTN_B");
    if (ioctl(fd, UI_SET_KEYBIT, BTN_X) < 0) perror("UI_SET_KEYBIT BTN_X");
    if (ioctl(fd, UI_SET_KEYBIT, BTN_Y) < 0) perror("UI_SET_KEYBIT BTN_Y");

    // Additional buttons
    if (ioctl(fd, UI_SET_KEYBIT, BTN_TL) < 0) perror("UI_SET_KEYBIT BTN_TL");
    if (ioctl(fd, UI_SET_KEYBIT, BTN_TR) < 0) perror("UI_SET_KEYBIT BTN_TR");
    if (ioctl(fd, UI_SET_KEYBIT, BTN_TL2) < 0) perror("UI_SET_KEYBIT BTN_TL2");
    if (ioctl(fd, UI_SET_KEYBIT, BTN_TR2) < 0) perror("UI_SET_KEYBIT BTN_TR2");
    if (ioctl(fd, UI_SET_KEYBIT, BTN_START) < 0) perror("UI_SET_KEYBIT BTN_START");
    if (ioctl(fd, UI_SET_KEYBIT, BTN_SELECT) < 0) perror("UI_SET_KEYBIT BTN_SELECT");
    if (ioctl(fd, UI_SET_KEYBIT, BTN_THUMBL) < 0) perror("UI_SET_KEYBIT BTN_THUMBL");
    if (ioctl(fd, UI_SET_KEYBIT, BTN_THUMBR) < 0) perror("UI_SET_KEYBIT BTN_THUMBR");
    if (ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_UP) < 0) perror("UI_SET_KEYBIT BTN_DPAD_UP");
    if (ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_DOWN) < 0) perror("UI_SET_KEYBIT BTN_DPAD_DOWN");
    if (ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_LEFT) < 0) perror("UI_SET_KEYBIT BTN_DPAD_LEFT");
    if (ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_RIGHT) < 0) perror("UI_SET_KEYBIT BTN_DPAD_RIGHT");

    // Enable absolute axis events for joystick movement.
    if (ioctl(fd, UI_SET_EVBIT, EV_ABS) < 0) perror("UI_SET_EVBIT EV_ABS");
    setup_abs(fd, ABS_X, -512, 512);
    setup_abs(fd, ABS_Y, -512, 512);

    // Set up the uinput device
    struct uinput_setup usetup = {
        .name = "Minimal B Sequence",
        .id = {
            .bustype = BUS_USB,
            .vendor  = 0x3,
            .product = 0x3,
            .version = 2,
        }
    };

    if (ioctl(fd, UI_DEV_SETUP, &usetup) < 0) {
        perror("UI_DEV_SETUP");
        return 1;
    }
    
    if (ioctl(fd, UI_DEV_CREATE) < 0) {
        perror("UI_DEV_CREATE");
        return 1;
    }

    // 0. Wait user switch back to GI
    char input_ws[4];
    int ws;
    char *eptr;
    printf("Enter an integer of seconds before switching back to GI\n");
    printf("Or start sequence now: ");
    if (fgets(input_ws, sizeof(input_ws), stdin)) {
        ws = strtol(input_ws, &eptr, 10);
        if (input_ws != eptr && *eptr == '\n') {
            sleep(ws);
        }
    }

    // 1. B click
    simulate_click(fd, BTN_B);
    msleep(500);

    // 2. B click (again)
    simulate_click(fd, BTN_B);
    msleep(500);

    // 3. Set Y axis to -512
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_ABS;
    ev.code = ABS_Y;
    ev.value = -512;
    if (write(fd, &ev, sizeof(ev)) < 0)
        perror("write ABS_Y");
    
    // Synchronize the axis update.
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    if (write(fd, &ev, sizeof(ev)) < 0)
        perror("write SYN after ABS_Y");
    msleep(500);

    // 4. B click (final)
    simulate_click(fd, BTN_B);
    msleep(500);

    // Clean up: destroy the device.
    if (ioctl(fd, UI_DEV_DESTROY) < 0)
        perror("UI_DEV_DESTROY");
    close(fd);
    
    return 0;
}
