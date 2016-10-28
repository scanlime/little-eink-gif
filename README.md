# Little Eink Gif

It's a simple animated image viewer and proof-of-concept for the 2.1" 72x172 pixel electrophoretic display (EPD) from Dalian Good Display Co.

These EPDs are a specific type of bi-stable display from the broad category known as e-paper or e-ink. These tiny EPDs are commonly manufactured for use in retail price signage, but you can buy modules and development kits from sites like aliexpress.

This display is model GDE021A1, and the controller claims to be SPD2701.

<img src="http://i.imgur.com/knPYs4O.jpg" width="100%" alt="Example Photo" />


## How to Compile

You need to install the following prerequisites:

- libopencm3 (a library that makes it easier to make ARM-based firmware)
- openocd (a command-line tool for flashing the firmware)
- gcc-arm-embedded (a gcc cross-compiler that lets you compile ARM-based software on a non-ARM-based platform)

To install libopencm3, simply initialize the git submodule, and update. This will install libopencm3 in the `./libopencm3` subdirectory of this project:

```
git submodule update --init
```

Next, run make inside the `libopencm3` directory:

```
(cd libopencm3; make)
```

To install `openocd` on Mac OS X, use brew:

```
brew install openocd
```

To install gcc-arm-embedded on Mac OS X, use brew:

```
brew install Caskroom/cask/gcc-arm-embedded
```

Before you can run `make` inside this project directory, make sure you ran `make` inside `libopencm3` above, or else you will get complaints from the compiler regarding missing header files such as `nvic.h`.

Finally, run make:

```
make
```

The output will be a .elf file. You can also run `make hex` and if you want to make the hex file *and* flash at the same time, `make flash`. To debug issues during the `make` process, try V=1, e.g. `make V=1 flash`.
