
from machine import UART, FPIOA

# 配置 GPIO44 → UART2_TXD, GPIO45 → UART2_RXD
fpioa = FPIOA()
fpioa.set_function(44, FPIOA.UART2_TXD)
fpioa.set_function(45, FPIOA.UART2_RXD)

class MotorController:
    def __init__(self, uart_port=UART.UART2, baudrate=115200):
        self.uart = UART(
            uart_port,
            baudrate=baudrate,
            bits=UART.EIGHTBITS,
            parity=UART.PARITY_NONE,
            stop=UART.STOPBITS_ONE
        )

    def rotate(self, motor_id, pwm, duration):
        cmd = "#"
        cmd += "{:03d}".format(motor_id)
        cmd += "P{:04d}".format(pwm)
        cmd += "T{:03d}!".format(duration)
        cmd += "\r\n"
        self.uart.write(cmd)

    def stop_all(self):
        for i in range(1, 5):
            self.rotate(i, 0, 0)

    def rotate_clockwise(self):
        self.rotate(1, 1000, 50)
        self.rotate(2, 1000, 50)
        self.rotate(3, 2000, 50)
        self.rotate(4, 2000, 50)

    def rotate_counterclockwise(self):
        self.rotate(1, 2000, 50)
        self.rotate(2, 2000, 50)
        self.rotate(3, 1000, 50)
        self.rotate(4, 1000, 50)

    def forward(self):
        self.rotate(1, 2380, 50)
        self.rotate(3, 830, 50)
        self.rotate(2, 830, 50)
        self.rotate(4, 2350, 50)

    def left_shift(self):
        self.rotate(2, 910, 40)
        self.rotate(1, 900, 40)
        self.rotate(4, 850, 40)
        self.rotate(3, 870, 40)

    def right_shift(self):
        self.rotate(1, 2000, 40)
        self.rotate(2, 1950, 40)
        self.rotate(4, 2040, 40)
        self.rotate(3, 1980, 40)
