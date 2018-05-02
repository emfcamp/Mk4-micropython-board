from machine import PWM

pwm = PWM(0)
pwm = PWM(0, freq=10000)
pwm = PWM(0, duty=50)
pwm = PWM(0, duty=50, freq=10000)
print(pwm.duty() == 50)
print(pwm.freq() == 10000)

pwm.duty(35)
print(pwm.duty() == 35)

pwm.duty(200)
print(pwm.duty() == 100)

pwm.freq(1000)
print(pwm.freq() == 1000)

pwm.init(freq=5000)
print(pwm.freq() == 5000)
print(pwm.duty() == 100)

pwm.init(duty=25)
print(pwm.freq() == 5000)
print(pwm.duty() == 25)

pwm.init(duty=66, freq=8000)
print(pwm.freq() == 8000)
print(pwm.duty() == 66)

pwm.duty(0)
