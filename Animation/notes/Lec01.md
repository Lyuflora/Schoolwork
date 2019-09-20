#计算机动画
秋冬学期
#Lecture 01 运动路径控制
##最简单的：线性插值
###路径
P(u) = (1-u) × P0 + u × P1

![](https://i.imgur.com/1FNLPRZ.png)

矩阵形式：

![](https://i.imgur.com/Mj19zEB.png)

在始末两点(P0, P1)之间生成插值的点，横坐标均匀分布。假设在两点之间有a, b, c, d四个插值位置，则它们的成分为：

X = (1-t) × x0 + t × x1

Y = (1-t)× y0 + t × y1

  t的取值依次为: 0.2, 0.4, 0.6, 0.8

以向量的角度看，在\vec P0P1上取点，保持坐标在线段上。

###速度
上述的t被替换成了关于t的函数law，也就是改变了等时间间隔插值点之间的距离，而这接近于速度的定义。保持插值频率，只要插值点的横坐标变得紧密或者疏散，就达到了“变速”的效果。

    1. 等速:  law=t;
    2. 加速:  law=(1-cos(PI*t/2));
    3. 减速:  law=sin(PI*t/2);
   
代码：

	    for(t=0, t<=1,t=t+𝛥t) {
          xin=(1-law)*x0+law*x1 ，
          yin=(1-law)*y0+law*y1 ，
     }

---

##曲线Hermite插值
现在插值不再只根据始末两点的位置，而是结合了它们的切线，让曲线在给出点的附近形成理想的弧度。

 ![](https://i.imgur.com/3PZOAr2.png)

Hermite插值产生曲线，基于Hermite base，以下四个：

![](https://i.imgur.com/w197rMm.png) ![](https://i.imgur.com/L2NHFBA.png) ![](https://i.imgur.com/cKsUeCd.png) ![](https://i.imgur.com/FKy30SX.png)

		 h1(u) = 2u^3 – 3u^2 + 1 
         h2(u) = -2u^3 + 3u^2 
         h3(u) = u^3 – 2u^2 + u
         h4(u) = u^3 - u^2 


这四个三次曲线基函数构造出符合理想形状的插值曲线（也是三次曲线）。不过构造符合特定点切线要求的曲线结果并不唯一，引入参数т描述曲线的“柔软程度”。

![](https://i.imgur.com/sK7ebkc.jpg)

考虑参数т的情况，也就是考虑了剩余点的切线情况。

###Catmull 曲线
通常不会详细地知道所有点的切线，所以还要自己计算出一个合适的值(参数т的作用出现了)：

P'i = 1/2 × (Pi+1 - Pi-1)

![](https://i.imgur.com/k5OEii5.png)


现在可以合成曲线方程了。
已知条件：一系列点，和指定始末点的斜率。
如果不用给出始末点的斜率则更简洁，那么需要一个计算初始点切向量的方法——

自动计算.比如从第二个点P2减去P1,将其反向得到一个虚拟点,该虚拟点减去初始点作为初始点的切向量.

![](https://i.imgur.com/MtyS9rl.png)

P'0 = 1/2 × (P1-(P2-P1)-P0)

![](https://i.imgur.com/KCPQuj6.png)
![](https://i.imgur.com/bRV4vac.png)
![](https://i.imgur.com/9nu2Iu5.png)

相乘即可。
下面考虑参数т。

###Cardinal Spline

P’i = τ* ( Pi+1 - Pi-1 )

将Catmull曲线更一般化，中间点的切线计算可以用参数τ控制，让曲线更接近多个圆弧的组合或折线。

![](https://i.imgur.com/cHj2SdU.png)

###速度控制
