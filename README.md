# handleGISW
An emulated handle to switch off the handle

Developed upon https://github.com/GrantEdwards/uinput-joystick-demo

## Prerequesite
`gcc emu_handle.c -o emu_handle`

Need read/write permission on `/dev/uinput`

## Usage

1. 进入游戏
2. 命令行 `./emu_handle`
3. 设置->控制设备->键盘鼠标切换为手柄
4. 切换到命令行，按 s、回车完成操作
