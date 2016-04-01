#include <dt-bindings/gpio/gpio.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/pwm.h>
#include <linux/pwm_backlight.h>
#include <linux/slab.h>


int gpio;
int en_value;

static struct of_device_id led_of_match[] = {
	{ .compatible = "softwin_led" },
	{ }
};

MODULE_DEVICE_TABLE(of, led_of_match);


static int led_remove(struct platform_device *pdev)
{
	return 0;
}

static int led_suspend(struct device *dev)
{
	gpio_direction_output(gpio, !en_value);
	return 0;
}

static int led_resume(struct device *dev)
{
	gpio_direction_output(gpio, en_value);
	return 0;
}

static int led_fb_event_notify(struct notifier_block *self, unsigned long action, void *data)
{
        struct fb_event *event = data;
        int blank_mode = *((int *)event->data);

        if (action == FB_EARLY_EVENT_BLANK) {
                switch (blank_mode) {
                        case FB_BLANK_UNBLANK:
                                break;
                        default:
                                led_suspend(NULL);
                                break;
                }
        }
        else if (action == FB_EVENT_BLANK) {
                switch (blank_mode) {
                        case FB_BLANK_UNBLANK:
                                led_resume(NULL);
                                break;
                        default:
                                break;
                }
        }
        return NOTIFY_OK;
}

static struct notifier_block led_fb_notifier = {
        .notifier_call = led_fb_event_notify,
};


static int led_probe(struct platform_device *pdev)
{
        struct device_node *node = pdev->dev.of_node;
	enum of_gpio_flags flags;
	int ret;

	//printk("------ %s ------\n", __func__);
        if (!node)
                return -ENODEV;

	gpio = of_get_named_gpio_flags(node, "power_led", 0, &flags);
	if(!gpio_is_valid(gpio)){
		dev_err(&pdev->dev, "invalid power_led gpio%d\n", gpio);
	}

	ret = devm_gpio_request(&pdev->dev, gpio, "power_led_gpio");
	if (ret) {
		dev_err(&pdev->dev, "failed to request GPIO%d for power_led\n", gpio);
		return -EINVAL;
	}
	en_value = (flags == GPIO_ACTIVE_HIGH)? 1:0;
	gpio_direction_output(gpio, en_value);

	fb_register_client(&led_fb_notifier);

        return 0;
}

static struct platform_driver led_driver = {
	.driver		= {
		.name		= "softwin_led",
		.owner		= THIS_MODULE,
		.of_match_table	= of_match_ptr(led_of_match),
	},
	.probe		= led_probe,
	.remove		= led_remove,
};

static int __init led_init(void)
{
	return platform_driver_register(&led_driver);
}

static void __exit led_exit(void)
{
	platform_driver_unregister(&led_driver);
}

//module_init(led_init);
fs_initcall(led_init);
module_exit(led_exit);

MODULE_AUTHOR("Samty");
MODULE_DESCRIPTION("SOFTWIN LED Driver");
MODULE_LICENSE("GPL");
