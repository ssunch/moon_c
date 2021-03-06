#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>      /* LINUX_VERSION_CODE & KERNEL_VERSION() */
#include <linux/platform_device.h>                                              
#include <linux/export.h>                                              
#include <linux/property.h>                                                     
#include <linux/interrupt.h>

/**************************************************************************
 * d3 device attribute
 **************************************************************************/
int d3 = 0;

static ssize_t d3_show(struct device_driver *driver,  
		char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", d3);
}

static ssize_t d3_store(struct device_driver *driver,
		const char *buf, size_t len)
{
	sscanf(buf, "%d", &d3);
	return sizeof(int);
}

static DRIVER_ATTR_RW(d3);
static struct attribute *drv3_driver_attrs[] = {
	&driver_attr_d3.attr,
	NULL
};
ATTRIBUTE_GROUPS(drv3_driver);


/**************************************************************************
 * drv3 driver
 **************************************************************************/

struct foo_data {
	struct platform_device *dev;
	int irq;		/* virq */
	void __iomem *base;	/* virtual address of device register */
};

static irqreturn_t foo_irq_handler(int irq, void *data)
{
        printk("%s\n", __func__);

	return IRQ_HANDLED;
}

static int drv3_probe(struct platform_device *pdev)
{ 
	int ret = 0;
	struct foo_data *foo;
	struct resource *res;

	dev_info(&pdev->dev, "%s\n", __func__);

	/* step 1) alloc driver data */
	foo = devm_kzalloc(&pdev->dev, sizeof(*foo), GFP_KERNEL);
	if (!foo)                                                         
		return -ENOMEM; 

	foo->dev = pdev;
	platform_set_drvdata(pdev, foo);

	/* step 2) get platform resource */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "platform mem resources not found.\n");
		return -EINVAL;
	}

        dev_info(&pdev->dev, "iomem resource. start=0x%lx, size=0x%lx\n", 
			(long unsigned int) res->start, 
			(long unsigned int) (res->end - res->start + 1));

	/* step 3) ioremap */
	foo->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(foo->base)) {
		dev_err(&pdev->dev, "devm_ioremap_resource() failed.\n");
		return -ENOMEM;
	}
	
        dev_info(&pdev->dev, "iomem resource. base=0x%p\n", foo->base);

	/* step 4) get irq */
	foo->irq = platform_get_irq(pdev, 0);
	if (foo->irq < 0) {
		dev_err(&pdev->dev, "platform irq resources not found.\n");
		return foo->irq;
	}
	dev_info(&pdev->dev, "irq=%d\n", foo->irq);

	/* step 5) request irq */
	ret = devm_request_threaded_irq(&pdev->dev, foo->irq,
			NULL, foo_irq_handler,
			IRQF_SHARED | IRQF_ONESHOT,                             
			"drv4", foo);
	if (ret) {
		dev_err(&pdev->dev, 
				"devm_request_threaded_irq() failed. err=%d\n",
				ret);
		return ret;
	}
	
	dev_info(&pdev->dev, "devm_request_threaded_irq() irq=%d, ret=%d\n",
			foo->irq, ret); 

	return 0;
} 

static const struct platform_device_id drv3_id_table[] = {                      
	{ "foo3", 0 },
	{ },
};                                                                              

static const struct of_device_id drv3_of_match_table[] = { 
    { 
        .compatible = "foo,foo3", 
    }, 
    { /* sentinel */ }, 
}; 
MODULE_DEVICE_TABLE(of, drv3_of_match_table);

static struct platform_driver drv3 = {
	.probe = drv3_probe,
	.id_table = drv3_id_table,
	.driver = {
		.name = "drv3",
		.groups = drv3_driver_groups,
		.of_match_table = drv3_of_match_table,
	},
};                                                                              

module_platform_driver(drv3);
MODULE_LICENSE("GPL");

