# 媒体资源管理

## Lecture2
## Image encoding 图像的编码
有损 或 无损
### Run-Length Encoding 游程编码
### LZW 解码和编码
Universal lossless data compression algorithm

有损压缩Lossless的常用方法：
<br>
**Quantization 量化**<br>
**Transform coding 变换编码**
<br><br>

### JPEG图像压缩

参考“媒体信号处理”笔记：
>
1. RGB无损转化为YUV
2. 颜色下采样
3. 图像切分为8×8的子块，DCT变换得到频域图像
4. 量化，使用不同类型的**量化表**对亮度层和色彩图像量化（相除取整），越靠近矩阵右下角，数值的损失越多。
5. ZigZag展开，低频部分在前
6. AC系数RLE编码
7. DC系数编码，适合差分编码
8. 熵编码

当中有一步量化，会将接近0的值简单归为0，产生了损失。

**Decode**

1. 熵解码
2. 反量化
3. 反DCT变换
4. 合并

### GIF
Graphics Interchange Fotmat

256种颜色，色域较窄，使用LZW编码。
支持多帧动画，支持透明背景图像，并且文件小，下载速度快，可用许多具有同样大小的图像文件组成动画
###PNG
Portable Network Graphics

支持完整的透明通道

### TIF
包含标签信息，比如相机属性。采用无损压缩，支持多种色彩图像模式。是一种非常好的图像格式。

### DNG
**重点**<br>
分辨率
压缩率
bit per pixel
<br>
---
## 音频

重要的概念：量化，允许非均匀量化，而不是简单的抹去小数点<br>
Digitalized Audio/ Sound
数字化

**Bit rate 位率 采样率**<br>
一秒之内的比特数。考虑人耳性能，和Nyquist定理，信号采集到一定频率足够。

**如何理解有损和无损？**
不可能有完全的无损，即使从RGB转YUV也有损失。在音频中，PCM

**PCM编码：Pulse Code Modulation脉冲编码调制**<br>
>将声音等模拟信号变成符号化的脉冲列，再予以记录。PCM信号是由[1]、[0]等符号构成的数字信号，而未经过任何编码和压缩处理。与模拟信号比，它不易受传送系统的杂波及失真的影响。动态范围宽，可得到音质相当好的影响效果。
高质量，大体积。

**Why 72?**
早期磁带录制约560分钟，PCM是双通道，采样4w+，16bit，换算一分钟对应10M存储容量，一般不超过700M(大多600M+)，取其上限可放置72分钟。

**容量计算**
128k 和带宽有关的计算

**streaming feature of audio**
支持一边播放一边传输，有损<br>
常用音频格式:WAV MP3 WMA(微软，流式) RA(流式) OGG APE

### WAV
### RA
有分块存储的需要(时间)

技术上WAV, RA两者性能都比较好，但大多数使用的是mp3。<br>

### APE
适合无损压缩的格式**APE**，使用**LZW**方法

### MP3
MPEG-3，主流格式
使用了类似于图像压缩的**频域压缩**的原理，压缩高频信息，并利用人的感知原理——当人在听音乐时，注意力集中在人声部分而不是重叠的背景声。
>
高效率的编码在于去掉冗余信号以及人的听音系统并不能感受到的频率范围中的不必要的信号。
所有的编码器用相同的基本结构。编码方案可以描述成“感知噪音形态”或“感知子带（subband）/变换编码”。编码器通过对声音信号的光谱组成分析，计算出转换的过滤带，并且通过一个心理分析模型，来估计出刚好能被人感知的噪音的级别。

### OGG
一种比较有希望成为主流的格式，Google和一些Linux厂家支持。

**什么是MIDI**<br>
1. 一种音乐标准
2. 一套通讯协议(在设备、乐器之间)
3. 具有MIDI演奏、回放能力的乐器

组成部分：
1. Synthesizer 合成器
2. Sequencer 序列器，设备帮助存储信息
3. MIDI device支持MIDI的设备，不过声音实际上不是在这里产生的

记录信息：
1. Track 通道 同一个乐器不同通道/同一时间多个声音的叠加/不同乐器/声音的不同频域、调...
2. Message 消息 可以定义Command..进行控制


Common MIDI Format<br>
MIDI SMF

<br>
---
<br>
## 视频

一个序列的，三维的图像？->不完全正确
常见视频格式：
AVI,
RM,
MOV,
MPEG(MPEG-1,MPEG-2,**MPEG-4**),

### AVI
Audio Video Interleaved
音视频叠加存储

>codec:
>不同的厂商公司在压缩方式上有自己的方案

### RM
一种流媒体视频文件格式

### MOV
###MPEG标准
`MPEG-1`:
>Coding of moving pictures and associated audio for digital storage media.视频压缩、解压缩、回放，电视<br>
>CD-1, DIGITAL MULTIMEDIA
>存储的，可建立数据库点播

<br>
`MPEG-2`:
>Coding of moving pictures and associated audio for digital storage media.主流的，面向电视的NTSC(美、日)<br>
>VCR功能（录、快放等）

<br>
`MPEG-4`
>互联网影视传播

`MPEG-7`<br>
`MPEG-21`  媒体资源管理

**MPEG**: 只压缩运动部分
>only compress moving parts

帧间的连贯性：
`I B B P` `I B B P`  `I B B P`...
P帧——运动补偿<br>
B帧——插值，B之前的I和B之后的P加权平均<br>
I帧——压缩
<br><br>

**Color video Compression**

下采样
**YUV420**等

###H.26X
H.265

