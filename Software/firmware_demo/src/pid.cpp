#include "pid.h"

PID::PID(double kp, double ki, double kd, double dt)
{
    m_kp = kp;
    m_ki = ki;
    m_kd = kd;
    m_previous_error = 0;
    m_integral = 0;
    m_dt = dt;
}

PID::~PID()
{
    // 释放资源
}

/******************************************************************
 * 设置PID参数
 */
void PID::set_pid_param(double kp, double ki, double kd, double dt)
{
    m_kp = kp;
    m_ki = ki;
    m_kd = kd;
    m_dt = dt;
}

void PID::set_data(double pre_error, double integral)
{
    m_previous_error = pre_error;
    m_integral = integral;
}

double PID::get_output(double target_value, double now_value)
{
    double error = target_value - now_value;
    m_integral = m_integral + error * m_dt;
    double derivative = (error - m_previous_error) / m_dt;
    double output = m_kp * error + m_ki * m_integral + m_kd * derivative;
    m_previous_error = error;
    return output;
}