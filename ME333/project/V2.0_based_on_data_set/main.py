from libs.PipeLine import PipeLine, ScopedTiming
from libs.YOLO import YOLOv5
import os, sys, gc
import ulab.numpy as np
import image
from machine import UART

if __name__ == "__main__":
    # 显示模式，默认"hdmi"，可选"lcd"
    display_mode = "lcd"
    rgb888p_size = [800, 480]
    if display_mode == "hdmi":
        display_size = [1920, 1080]
    else:
        display_size = [640, 480]

    # 模型路径与标签
    kmodel_path = "/sdcard/examples/kmodel/best.kmodel"
    labels = ["Golf"]  # 检测目标类别名
    confidence_threshold = 0.8
    nms_threshold = 0.45
    model_input_size = [320, 320]

    # 初始化串口 UART1
    uart1 = UART(UART.UART1, baudrate=115200, bits=UART.EIGHTBITS,
                 parity=UART.PARITY_NONE, stop=UART.STOPBITS_ONE)

    # 初始化 PipeLine 和 YOLOv5
    pl = PipeLine(rgb888p_size=rgb888p_size, display_size=display_size, display_mode=display_mode)
    pl.create()
    yolo = YOLOv5(task_type="detect", mode="video", kmodel_path=kmodel_path, labels=labels,
                  rgb888p_size=rgb888p_size, model_input_size=model_input_size,
                  display_size=display_size, conf_thresh=confidence_threshold,
                  nms_thresh=nms_threshold, max_boxes_num=50, debug_mode=0)
    yolo.config_preprocess()

    try:
        while True:
            os.exitpoint()
            with ScopedTiming("total", 1):
                # 获取图像帧
                img = pl.get_frame()
                # 运行模型
                res = yolo.run(img)

                # 如果检测到目标
                if len(res) > 0:
                    # 选取最大面积目标（确保准确性）
                    best = max(res, key=lambda b: b[2] * b[3])  # b = [x, y, w, h, conf, cls_id]
                    x, y, w, h = best[:4]
                    cx = x + w // 2
                    img_width = rgb888p_size[0]

                    # 计算yaw角（水平视角假设为60度）
                    FOV = 60
                    yaw = (cx - img_width / 2) / (img_width / 2) * (FOV / 2)

                    # 打印并通过串口发送
                    print("目标Yaw角: {:.2f}°".format(yaw))
                    uart1.write("%.2f\r\n" % yaw)

                # 显示检测结果
                yolo.draw_result(res, pl.osd_img)
                pl.show_image()

                # 清理内存
                gc.collect()
    except Exception as e:
        sys.print_exception(e)
    finally:
        yolo.deinit()
        pl.destroy()
