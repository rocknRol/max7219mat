#cython: language_level=3, boundscheck=False

cdef extern from "libmax7219mat.h":

    int initialize_library();
    int selftest_display();
    int check_display_module(int m, int t);
    int set_brightness(int b, int m);
    int render_line(int m, int l);
    int render_module(int m);
    int render_screen();
    int auto_render_enable(int v);
    int screen_clipping_enable(int v);
    int set_pixel(int x, int y, int v);
    int get_pixel(int x, int y);
    int clear_screen();
    int quit_library();

def initializeLibrary() -> int:
    initialize_library()

def selftestDisplay() -> int:
    selftest_display()

def checkDisplayModule(m: int, t: int) -> int:
    check_display_module(m, t)

def setBrightness(b: int, m: int) -> int:
    set_brightness(b, m)

def renderLine(m: int, l: int) -> int:
    render_line(m, l)

def renderModule(m: int) -> int:
    render_module(m)

def renderScreen() -> int:
    render_screen()

def autoRenderEnable(v: int) -> int:
    auto_render_enable(v)

def screenClippingEnable(v: int) -> int:
    screen_clipping_enable(v)

def setPixel(x: int, y: int, v: int) -> int:
    set_pixel(x, y, v)

def getPixel(x: int, y: int) -> int:
    get_pixel(x, y)

def clearScreen() -> int:
    clear_screen()

def quitLibrary() -> int:
    quit_library()
