Keywords:FreeRTOS, STM32, Key Combo Detection
# 模块介绍
  基于STM32，使用FreeRTOS设计了一个按键检测模块，该模块支持两种输入模式：  
  1. INPUT_MODE_NORMAL 普通输入模式，支持检测长按、短按
  2. INPUT_MODE_COMMAND 命令输入模式，L表示长按，S表示短按，该模式下可支持不同输入组合，如LLL（连续三次长按），SS（连续两次短按），可自定义回调函数  
  模块中的头文件对每一项都进行了详细的注释  
  examples中的例子使用了一个LED灯、一个蜂鸣器来验证模块的功能  
