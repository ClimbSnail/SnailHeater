#ifndef _PID_H_
#define _PID_H_

class PID
{
public:
    double m_kp;
    double m_ki;
    double m_kd;
    double m_previous_error;
    double m_integral;
    double m_dt;

public:
    PID(double kp, double ki, double kd, double dt);
    ~PID();
    void set_pid_param(double kp, double ki, double kd, double dt);
    void set_data(double pre_error, double integral);
    double get_output(double target_value, double now_value);
};

#endif