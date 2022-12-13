/*
 * gpio.c
 * 
 * Written & released by Keir Fraser <keir.xen@gmail.com>
 * 
 * This is free and unencumbered software released into the public domain.
 * See the file COPYING for more details, or visit <http://unlicense.org>.
 */

#define m(bitnr) (1u<<(bitnr))

#define U_BUF_SZ 1024
static uint8_t u_buf[1024];

static enum {
    ST_inactive,
    ST_command_wait,
} gpio_state = ST_inactive;

static uint32_t u_prod;

static void gpio_reset(void)
{
    gpio_state = ST_inactive;
}

/* Status: sa9 
 * Set output: ha9, la9
 * Set input: ua9, da9, fa9
 */

static void process_command(uint32_t p)
{
    int cmd, pin;
    volatile struct gpio *gpio;

    if (p < 3)
        goto err;

    cmd = u_buf[0];

    switch (u_buf[1]) {
    case 'a': gpio = gpioa; break;
    case 'b': gpio = gpiob; break;
    case 'c': gpio = gpioc; break;
    default: goto err;
    }

    pin = u_buf[2] - '0';
    if ((p >= 4) && (u_buf[3] >= '0') && (u_buf[3] <= '9'))
        pin = pin*10 + (u_buf[3] - '0');
    if (pin < 0 || pin > 15)
        goto err;

    switch (cmd) {
    case 'h':
        gpio_configure_pin(gpio, pin, GPO_pushpull(IOSPD_LOW, HIGH));
        break;
    case 'l':
        gpio_configure_pin(gpio, pin, GPO_pushpull(IOSPD_LOW, LOW));
        break;
    case 'f':
        gpio_configure_pin(gpio, pin, GPI_floating);
        break;
    case 'u':
        gpio_configure_pin(gpio, pin, GPI_pull_up);
        break;
    case 'd':
        gpio_configure_pin(gpio, pin, GPI_pull_down);
        break;
    case 's':
        break;
    default:
        goto err;
    }

    u_buf[0] = gpio_read_pin(gpio, pin) + '0';
    usb_write(EP_TX, u_buf, 1);
err:
    return;
}

static void gpio_configure(void)
{
    gpio_state = ST_command_wait;
    u_prod = 0;
}

void gpio_process(void)
{
    int len, cmd;
    uint32_t c, p;

    if (gpio_state != ST_command_wait)
        return;

    len = ep_rx_ready(EP_RX);
    if (len < 0)
        return;

    p = u_prod;
    usb_read(EP_RX, &u_buf[p], len);
    p += len;

    for (c = 0; c < p; c++) {
        cmd = u_buf[c];
        if (cmd >= 'a' && cmd <= 'z')
            break;
    }

    if (c != 0) {
        if (c != p)
            memmove(&u_buf[0], &u_buf[c], p-c);
        p -= c;
    }

    if ((p != 0) && (u_buf[p-1] == '\n')) {
        process_command(p);
        p = 0;
    }

    /* Arbitrary clamp. */
    u_prod = min_t(uint32_t, p, 100);
}

const struct usb_class_ops usb_cdc_acm_ops = {
    .reset = gpio_reset,
    .configure = gpio_configure
};

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
