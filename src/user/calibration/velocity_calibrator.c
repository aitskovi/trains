#include <velocity_calibrator.h>

#include <dassert.h>
#include <encoding.h>
#include <pubsub.h>
#include <syscall.h>
#include <nameserver.h>
#include <clock_server.h>

#define NUM_INNER_SWITCHES 5
#define MIN_SPEED 4
#define MAX_SPEED 14
#define NUM_SAMPLES 10

void velocity_calibrator(int train) {
    tid_t mission_control = WhoIs("MissionControl");

    int switches[NUM_INNER_SWITCHES];
    switches[0] = 6;
    switches[1] = 8;
    switches[2] = 9;
    switches[3] = 14;
    switches[4] = 15;
    int i;
    for (i = 0; i < NUM_INNER_SWITCHES; ++i) {
        SetSwitch(switches[i], CURVED);
    }

    for (i = MIN_SPEED; i <= MAX_SPEED; ++i) {
        // Send the train off at that speed.
        Message msg, rply;
        msg.type = SHELL_MESSAGE;
        ShellMessage *sh_msg = &msg.sh_msg;
        sh_msg->type = SHELL_SET_TRAIN_SPEED;
        sh_msg->speed = i;
        sh_msg->train_no = train;
        Send(mission_control, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));
        cuassert(rply.type == SHELL_MESSAGE, "Invalid Reply");
        cuassert(rply.sh_msg.type == SHELL_SUCCESS_REPLY, "Not successful shell reply");

        // Delay for 400 ticks so the train speeds up.
        Delay(400);

        // Subscribe to location updates.
        Subscribe("CalibrationServerStream", PUBSUB_MEDIUM);

        // Get average speed over 8 sensor edges.
        int total_velocity = 0;
        int num_samples = 0;

        int tid;
        do {
            // Recieve a location Message.
            Receive(&tid, (char *) &msg, sizeof(msg));
            cuassert(msg.type == CALIBRATION_MESSAGE, "Invalid Calibrator Message");
            cuassert(msg.cs_msg.type == CALIBRATION_INFO, "Invalid Calibrator Message");

            // Ack Calibration Message.
            rply.type = CALIBRATION_MESSAGE;
            rply.cs_msg.type = CALIBRATION_INFO;
            Reply(tid, (char *) &rply, sizeof(rply));

            if (msg.cs_msg.train != train) continue;

            total_velocity += msg.cs_msg.velocity;
            num_samples++;
        } while(num_samples < NUM_SAMPLES);

        /*
        sh_msg->type = SHELL_SET_TRAIN_SPEED;
        sh_msg->speed = 0;
        sh_msg->train_no = train;
        Send(mission_control, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));
        cuassert(rply.type == SHELL_MESSAGE, "Invalid Reply");
        cuassert(rply.sh_msg.type == SHELL_SUCCESS_REPLY, "Not successful shell reply");
        */

        // Unsubscribe from location updates.
        Unsubscribe("CalibrationServerStream", PUBSUB_MEDIUM);
        
        // Print average speed.
        ulog("Train %d, Average Speed: %d", train, total_velocity / num_samples);
    }

    Exit();
}
