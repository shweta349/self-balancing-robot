#define DEBUG 0

#include <Arduino.h>
#include <stdint.h>
#include "PID_v1.h"
#include "printf.h"
#include "utils.h"

double pid_chain_setpoint_angle, pid_chain_input_angle, pid_chain_output_angle;
double pid_chain_setpoint_speed, pid_chain_input_speed, pid_chain_output_accel;

// pid to obtain the desired angle given the current speed and the target speed
PID pid_chain_controller_angle(&pid_chain_input_speed, &pid_chain_output_angle, &pid_chain_setpoint_speed, 0.1, 0.1, 0.1, DIRECT);
// pid to obtain the desired speed given the current angle and the desired angle
PID pid_chain_controller_speed(&pid_chain_input_angle, &pid_chain_output_accel, &pid_chain_setpoint_angle, 2, 0.00, 0.2, DIRECT);

void setup_pid_chain_chain() {
  pid_chain_controller_angle.SetMode(AUTOMATIC);
  pid_chain_controller_angle.SetOutputLimits(-35, 35);

  pid_chain_controller_speed.SetMode(AUTOMATIC);
  pid_chain_controller_speed.SetOutputLimits(-40, 40);
}

//  * Get the current angle from the MPU
//  * Estimate the current speed
//  * Feed the current speed to the angle PID to obtain the desired angle to get to the input speed
//  * Feed the current angle to the speed PID to obtain the desired acceleration to get to the desired angle
//  * Adjust motor speeds accordingly
void get_pid_chain_motor_speed(int16_t * motor_accel, float angle, float angle_old, int16_t m1, int16_t m2) {
  // for now our target speed **ALWAYS** is zero
  pid_chain_setpoint_speed = 0;
  // we need to predict our current speed for the inputs we have
  int angular_velocity = (angle - angle_old) * 90;
  pid_chain_input_speed = (m1 - m2) / 2 - (pid_chain_input_speed-angular_velocity);

  pid_chain_controller_angle.Compute();

  // PID given the target angle and the current angle
  // outputs the target speed
  pid_chain_setpoint_angle = pid_chain_output_angle; 
  pid_chain_input_angle = angle;
  pid_chain_controller_speed.Compute();

#if DEBUG
  runEvery(1000) {
    printsf(__func__, "Speed PID: setpoint: %f  input: %f output (acceleration): %f", pid_chain_setpoint_angle, pid_chain_input_angle, pid_chain_output_accel);
  }
#endif

  motor_accel[0] = -(int16_t)pid_chain_output_accel;
  motor_accel[1] = -(int16_t)pid_chain_output_accel;
}
