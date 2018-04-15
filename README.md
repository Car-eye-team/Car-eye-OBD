# Car-eye-OBD
car-eye OBD 子系统是用来实现车内系统的数据采集，采用808协议进行数据上传。是car-eye 开源团队车内系统的重要组成部分

# OBD 功能说明
      通过标准车载OBD协议获取车辆关键数据，如：发动机转速、车速、水温、电瓶电压以及故障信息等。并对这些数据进行算法处理获取更有价值的数据，如：里程、油耗、发动机工作状态参数。

# DTU 功能说明
      DTU主要负责透传功能，通过TCP socket与平台建立连接，将OBD的数据、GPS数据、G-senor数据以交通部布标808协议格式发送到平台，信号不好情况，自己缓存数据到flash，支持OTA远程升级功能。
      


# 联系我们

car-eye 开源官方网址：www.car-eye.cn; car-eye开源源码下载网址：https://github.com/Car-eye-team 有关car-eye 问题咨询可以加QQ群590411159。




