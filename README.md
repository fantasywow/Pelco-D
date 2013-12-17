Pelco-D
=======

An implemention of Pelco-D protocol in Windows using C++  
在windows下用C++实现的Pelco-D云台控制协议。  

Send command code by serial communication  
通过串口通信发送指定的命令码。 

Serial communication part is using the code from online.  
串口通信部分用的网上的代码。 

TODO:add speed control  
待完成:转动速度控制。  


PELCO-D命令参考：

数据格式：1位起始位、8位数据、1位停止位，无校验位。波特率：2400B/S    
命令格式：      
字节1    字节2   字节3   字节4   字节5   字节6   字节7    
同步字节 地址码  指令码1 指令码2 数据码1 数据码2 校验码  

1.该协议中所有数值都为十六进制数    
2.同步字节始终为FFH   
3.地址码为摄像机的逻辑地址号，地址范围：00H–FFH  
4.指令码表示不同的动作  
5.数据码1、2分别表示水平、垂直方向速度（00-3FH）,FFH表示“turbo”速度  
6.校验码 = MOD[（字节2 + 字节3 + 字节4 + 字节5 + 字节6）/100H]  
以地址码0x01为例：  
{0xff,0x01,0x00,0x08,0x00,0xff,0x08,}//上  
{0xff,0x01,0x00,0x10,0x00,0xff,0x10,}//下  
{0xff,0x01,0x00,0x04,0xff,0x00,0x04,}//左  
{0xff,0x01,0x00,0x02,0xff,0x00,0x02,}//右  
{0xff,0x01,0x00,0x20,0x00,0x00,0x21,}//变倍短  
{0xff,0x01,0x00,0x40,0x00,0x00,0x41,}//变倍长  
{0xff,0x01,0x00,0x80,0x00,0x00,0x81,}//聚焦近  
{0xff,0x01,0x01,0x00,0x00,0x00,0x02,}//聚焦远  
{0xff,0x01,0x02,0x00,0x00,0x00,0x03,}//光圈小  
{0xff,0x01,0x04,0x00,0x00,0x00,0x05,}//光圈大  
{0xff,0x01,0x00,0x0b,0x00,0x01,0x0d,}//灯光关   
{0xff,0x01,0x00,0x09,0x00,0x01,0x0b,}//灯光开  
{0xff,0x01,0x00,0x07,0x00,0x01,0x09,}//转至预置点001  
{0xff,0x01,0x00,0x03,0x00,0x01,0x05,}//设置预置点001  
{0xff,0x01,0x00,0x05,0x00,0x01,0x07,}//删除预置点001  
以上对应的停命令均是:  
{0xff,0x01,0x00,0x00,0x00,0x00,0x01,}//停命令   
