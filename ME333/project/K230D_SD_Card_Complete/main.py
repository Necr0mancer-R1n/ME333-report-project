
from libs.PipeLine import PipeLine, ScopedTiming 
from libs.YOLO import YOLOv5
from motor import MotorController
from machine import Pin, FPIOA
import os, sys, gc, time

# 设置 LED 引脚
fpioa = FPIOA()
fpioa.set_function(59, FPIOA.GPIO59)  # 蓝灯
fpioa.set_function(61, FPIOA.GPIO61)  # 红灯

ledb = Pin(59, Pin.OUT, pull=Pin.PULL_NONE, drive=7)
ledr = Pin(61, Pin.OUT, pull=Pin.PULL_NONE, drive=7)

def set_led(blue_on, red_on):
    ledb.value(1 if blue_on else 0)
    ledr.value(1 if red_on else 0)

set_led(0, 0)
print("系统启动中...")

display_mode = "lcd"
rgb888p_size = [800, 480]
display_size = [640, 480] if display_mode == "lcd" else [1920, 1080]
kmodel_path = "/sdcard/examples/kmodel/best.kmodel"
labels = ["Golf"]
confidence_threshold = 0.8
nms_threshold = 0.45
model_input_size = [320, 320]

pl = PipeLine(rgb888p_size=rgb888p_size, display_size=display_size, display_mode=display_mode)
pl.create()

yolo = YOLOv5(
    task_type="detect",
    mode="video",
    kmodel_path=kmodel_path,
    labels=labels,
    rgb888p_size=rgb888p_size,
    model_input_size=model_input_size,
    display_size=display_size,
    conf_thresh=confidence_threshold,
    nms_thresh=nms_threshold,
    max_boxes_num=10,
    debug_mode=0
)
yolo.config_preprocess()

motor = MotorController()

try:
    print("初始化完成，开始运行")
    set_led(1, 1)
    for i in range(3):
        set_led(1, 0)
        time.sleep(0.2)
        set_led(0, 1)
        time.sleep(0.2)
    set_led(0, 1)

    lost_counter = 0
    while True:
        os.exitpoint()
        with ScopedTiming("total", 1):
            img = pl.get_frame()
            res = yolo.run(img)
            yolo.draw_result(res, pl.osd_img)
            pl.show_image()
            gc.collect()

            if len(res) > 0:
                lost_counter = 0
                box = res[0]
                x_center = (box[0] + box[2]) / 2
                frame_center = display_size[0] / 2

                set_led(1, 0)

                if abs(x_center - frame_center) < 40:
                    motor.forward()
                elif x_center < frame_center:
                    motor.left_shift()
                else:
                    motor.right_shift()
            else:
                lost_counter += 1
                if lost_counter < 15:
                    set_led(0, 1)
                    motor.rotate_clockwise()
                else:
                    set_led(0, 0)
                    motor.stop_all()
except Exception as e:
    set_led(0, 0)
    for _ in range(10):
        ledr.value(1)
        time.sleep(0.1)
        ledr.value(0)
        time.sleep(0.1)
    sys.print_exception(e)
finally:
    motor.stop_all()
    yolo.deinit()
    pl.destroy()
