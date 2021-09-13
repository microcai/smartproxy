
# 汉语说明

## 起因

当用户有多条线路的时候， 如何自动选择最快的线路访问目标网站是个近乎不可能完成的任务。现有的方式是使用策略路由表。需要维护一个庞大的地址列表。并定期测试。
而且并不适用所有人。

不仅仅多运营商的情况下是如此，用户如果使用了额外的线路（如VPN）那么是使用直连更快呢？ 还是使用 VPN 访问更快呢？
这就变成了一个非常棘手的问题。

好在今天起，smartproxy 诞生了。smartproxy是一个自动选择最快路径的代理。如果使用多条线路，
smartproxy 同时在多条线路上发起对目标网站的连接，并选择速度最快的一个。

如果目标网站只能通过某条线路访问，则自动就进行了线路筛选！不需要为特定的网络编写巨大的代理地址列表(没错，我说的是 gfwlist）。

## 使用场景 1

最常见的场景是 使用一条 ISP 线路 + 一条 socks5 (socks5://127.0.0.1:1080 你懂得)代理。

这个场景下， smartproxy 无需配置， 直接运行即可。然后将浏览器的代理地址设定为 socks5://localhost:1810 即可。

目前会自动探测 socks5://127.0.0.1:1080 socks5://127.0.0.1:1081 socks5://127.0.0.1:1082 三条代理. 并使用最快的

## 使用场景 2

用户有2条 ISP 线路。第一条线路拨号后为 pppoe0, 第二条为 pppoe1，第三条为 一条 socks5 代理。

这个场景下， smartproxy 需要配置。

```json

{
	"upstreams" : [
		{
			"interface": "pppoe0"
		},
		{
			"interface": "pppoe1"
		},
		{
			"socks5" : "localhost",
			"socks5port": "1080"
		},
	]
}
```

TODO 配置文件说明

## 使用场景 3

用户有多条 socks5 代理。


```json

{
	"upstreams" : [
		{
			"socks5" : "localhost",
			"socks5port": "1080"
		},
		{
			"socks5" : "localhost",
			"socks5port": "1081"
		},
	]
}
```

## 使用场景 4

用户在路由器上拨号，设定192.168.1.30 流量走 pppoe0 , 192.168.1.40 流量走 pppoe1.

```json

{
	"upstreams" : [
		{
			"bind" : "192.168.1.30"
		},
		{
			"bind" : "192.168.1.40"
		},
	]
}
```


## 使用场景 5

用户使用了 VPN , eth0 为普通上网，
VPN 为 tun0 设备.

```json

{
	"upstreams" : [
		{
			"interface": "eth0"
		},
		{
			"interface": "tun0"
		},
	]
}
```

注意， 需要

	route add default eth0 gw 网关地址 metric 1
	route add default tun0 gw vpn的网关地址 metric 1000

把两个设备都加上默认路由， 只是 tun0 设备的 metirc 更大。上面 pppoe0/pppoe1 之类的设定都是同样的道理， 必须要有默认路由。但是用更大的 metric 把它日常排除。

