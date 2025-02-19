#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/cdrom.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <libudev.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>

struct device_event_info {
    char *action;
    char *devnode;
    char *devtype;
    char *media_cd;
    char *media_dvd;
    char *media_bd;
};

void free_dei(struct device_event_info *dei) {
    free(dei->action);
    free(dei->devnode);
    free(dei->devtype);
    free(dei->media_cd);
    free(dei->media_dvd);
    free(dei->media_bd);
    free(dei);
}

void startDetached(const char *program, char *const argv[]) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        // Parent exits immediately
        return;
    }

    // Child process
    if (setsid() < 0) {
        perror("setsid");
        exit(EXIT_FAILURE);
    }

    // Fork again to ensure the process cannot acquire a terminal again
    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Set file permissions
    umask(0);

    // Change working directory
    chdir("/");

    // Redirect standard file descriptors
    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);

    // Execute the program
    execvp(program, argv);

    // If execvp fails
    perror("execvp");
    exit(EXIT_FAILURE);
}

void *device_event(void *arg) {
    struct device_event_info *dei = (struct device_event_info *)arg;

    printf("Event: %s on device %s (%s)\n", dei->action, dei->devnode, dei->devtype);
    printf("Media Type: CD(%s), DVD(%s), BD(%s)\n", dei->media_cd, dei->media_dvd, dei->media_bd);

    const char *mediaType = "unknown";
    if (strcmp(dei->media_cd, "1") == 0) {
        mediaType = "cd";
    } else if (strcmp(dei->media_dvd, "1") == 0) {
        mediaType = "dvd";
    } else if (strcmp(dei->media_bd, "1") == 0) {
        mediaType = "bd";
    }

    if (strcmp(mediaType, "unknown") == 0) {
        printf("%s\n", "Unsupported media type");
        free_dei(dei);
        return NULL;
    }

    // Check action type
    if (strcmp(dei->action, "change") != 0) {
        printf("%s\n", "Unsupported action type");
        free_dei(dei);
        return NULL;
    }

    int fd = open(dei->devnode, O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        perror("Failed to open CD/DVD device");
        free_dei(dei);
        return NULL;
    }

    int status = ioctl(fd, CDROM_DRIVE_STATUS, CDSL_CURRENT);
    if (status == -1) {
        perror("ioctl CDROM_DRIVE_STATUS failed");
        close(fd);
        free_dei(dei);
        return NULL;
    }

    char *argDevice = (char *)calloc(1, 1024);
    strcpy(argDevice, "--device=");
    strcat(argDevice, dei->devnode);

    char *argMediaType = (char *)calloc(1, 1024);
    strcpy(argMediaType, "--mediaType=");
    strcat(argMediaType, mediaType);

    const char *args[] = {"autoplay-gui", argDevice, argMediaType, NULL};

    switch (status) {
        case CDS_NO_INFO:
            printf("Status: No information available\n");
            break;
        case CDS_NO_DISC:
            printf("Status: No disc inserted\n");
            break;
        case CDS_TRAY_OPEN:
            printf("Status: Tray is open\n");
            break;
        case CDS_DRIVE_NOT_READY:
            printf("Status: Drive not ready\n");
            break;
        case CDS_DISC_OK:
            printf("Status: Disc is inserted\n");
            startDetached(args[0], (char* const*)args);
            break;
        default:
            printf("Status: Unknown status\n");
            break;
    }

    free(argDevice);
    free(argMediaType);

    close(fd);
    free_dei(dei);

    return NULL;
}

int main() {
    struct udev *udev;
    struct udev_monitor *mon;
    struct udev_device *dev;

    // Create the udev object
    udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Cannot create udev object.\n");
        return 1;
    }

    // Set up a monitor to listen for events on block devices
    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "block", "disk");
    udev_monitor_enable_receiving(mon);

    printf("Listening for CD/DVD events...\n");

    while (1) {
        // Wait for a device event
        dev = udev_monitor_receive_device(mon);
        if (!dev) {
            continue;
        }

        const char *action = udev_device_get_action(dev);
        const char *devnode = udev_device_get_devnode(dev);
        const char *devtype = udev_device_get_devtype(dev);
        const char *media_cd = udev_device_get_property_value(dev, "ID_CDROM_MEDIA_CD");
        const char *media_dvd = udev_device_get_property_value(dev, "ID_CDROM_MEDIA_DVD");
        const char *media_bd = udev_device_get_property_value(dev, "ID_CDROM_MEDIA_BD");

        // Check if the event involves a CD/DVD drive
        if (action && devnode && devtype) {
            struct device_event_info *dei = (struct device_event_info *)calloc(1, sizeof(struct device_event_info));

            dei->action = strdup(action);
            dei->devnode = strdup(devnode);
            dei->devtype = strdup(devtype);
            dei->media_cd = (media_cd == NULL) ? strdup("0") : strdup(media_cd);
            dei->media_dvd = (media_dvd == NULL) ? strdup("0") : strdup(media_dvd);
            dei->media_bd = (media_bd == NULL) ? strdup("0") : strdup(media_bd);

            pthread_t thread;
            if (pthread_create(&thread, NULL, device_event, dei) != 0) {
                perror("Failed to create thread");
                free_dei(dei);
            }
        }
        udev_device_unref(dev);
    }

    udev_unref(udev);

    return 0;
}
