# STemWin-MemoryDevice
有时候我们只需要更新 LCD 的一小部分内容，如指针，针对这种情况， emwin 提供了另外
一种解决方法——自动设备对象。自动设备对象以分段存储设备为基础，对于一次仅更新显示
器一小部分的应用而言可能更高效。该设备会自动识别包含固定对象的显示器区域，以及包含
必须更新的移动或更改对象的区域。第一次调用绘制函数时，会绘制所有项目，每一次后续调
用都将仅更新移动或更改对象使用的空间，实际绘制操作使用分段存储设备，但仅限在必要空
间内。使用自动设备对象的主要优势  （相对于使用分段存储设备而言）是计算时间减少，因为
它不始终更新整个显示器.
