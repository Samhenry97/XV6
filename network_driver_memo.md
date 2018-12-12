#xv6 SigDev Networking Report
#### Stephen Lemp
#### Jared Messer
#### CpS360
#### 4/28/17

##Introduction
The ultimate goal of this project is to add networking support to the xv6 toy OS running on QEMU. This report outlines the components that would be necessary for such functionality and provides some direction on the implementation of those components.

##Virtio-net Driver
The main component necessary for such functionality is the virtio-net driver. This driver provides a layer of virtualization to the guest OS for it to interact with the underlying network card. It is basically an API for virtual I/O. Virtio-net talks to a driver called vhost-net on the host machine by poking special registers and buffers to send data.

![](http://dpdk.org/doc/guides-1.8/_images/virtio_linux_vhost.png)

Vhost runs on the host side. It allows QEMU “to offload the servicing of virtio-net devices to the vhost-net kernel module, reducing the context switching and packet copies in the virtual dataplane.” It is what virtio-net talks to and what bridges the gap between virtio-net and the physical NIC.
These two components are necessary for communication to happen across machines. We will now look at some of the setup required.

##Host Side
<i>It is still unclear as to what exactly is required here</i>

The host must have vhost installed and running.

##QEMU Side
must be notified of the vhost-net and virtio-net drivers and told how to interact with them. It must share the following information with the vhost-net module through the vhost-net API:

* The layout of the guest memory space, to enable the vhost-net module to translate addresses.
* The locations of virtual queues in QEMU virtual address space, to enable the vhost module to read/write directly to and from the virtqueues.
* An event file descriptor (eventfd) configured in KVM to send interrupts to the virtio- net device driver in the guest. This enables the vhost-net module to notify (call) the guest.
* An eventfd configured in KVM to be triggered on writes to the virtio-net device’s Peripheral Component Interconnect (PCI) config space. This enables the vhost-net module to receive notifications (kicks) from the guest.

##xv6 Side
###Implementing the virtio driver
Everything that belongs in this section is found in the first resource found in the Resources section [VirtIO sample code driver]. That resource describes in detail how Patrick Dumais implemented a virtio network driver for his Toy OS, and would likely be able to guide anyone through how to implement a virtio driver for xv6, though it does include some functions that are not shown that the author wrote for his OS.

###Resources
[VirtIO sample code driver](http://www.dumais.io/index.php?article=aca38a9a2b065b24dfa1dee728062a12)
http://wiki.osdev.org/Virtio
[Linux Network Drivers](http://www.xml.com/ldd/chapter/book/ch14.html)
[Loading Block Drivers](http://www.xml.com/ldd/chapter/book/ch12.html)
[Integrating QEMU with vhost](http://dpdk.readthedocs.io/en/v16.04/sample_app_ug/vhost.html)
[QEMU vhost driver](http://blog.vmsplice.net/2011/09/qemu-internals-vhost-architecture.html)
[Information on Virtio](http://www.linux-kvm.org/images/d/dd/KvmForum2007%24kvm_pv_drv.pdf)