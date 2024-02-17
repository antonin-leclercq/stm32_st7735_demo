import sys
from PIL import Image
from numpy import asarray

SOURCE_FILE_RPATH = "../app/src/"
SOURCE_FILE_NAME = "st7735_frame.c"
SOURCE_FILE_COMMENTS = ["// Make sure these defines match with the ones in \"st7735.h\"\n"]
SUPPORTED_SIZE = [(128, 160), (132, 162)]

IMG_FILE_NAME = "smiley.png"

PIXEL_WIDTH = SUPPORTED_SIZE[0][0]
PIXEL_HEIGHT = SUPPORTED_SIZE[0][1]

HELP = "usage : python frame_gen.py [options]\n" \
        "with options being :\n" \
        "\t-w <frame width> : set frame width in pixel (default 128)\n" \
        "\t-h <frame height> : set frame height in pixel (default 160) \n" \
        "\t-i <path> : fill frame with data from image located at <path>\n" \
        "\t-f <format> : set RGB format(for example 444, 565 or 666)\n"\
        "\t--help : display this help message\n"

def is_supported(width_height: str, size: int) -> bool:
    k = (width_height == "height")
    for i in range(len(SUPPORTED_SIZE)):
        if size == SUPPORTED_SIZE[i][k]:
            return True;
    return False
    

def parse_sysargs() -> None:
    global IMG_FILE_NAME, PIXEL_WIDTH, PIXEL_HEIGHT
    argc = len(sys.argv)
    if argc == 1:
        print("No arguments specified")
        print(HELP)
        return
        
    for i in range(1, argc):
        # Show help
        if sys.argv[i] == "--help":
            print(HELP)
            exit(0)
            
        # Specify display width
        if sys.argv[i] == "-w" and i < argc - 1:
            size = int(sys.argv[i + 1])
            if is_supported("width", size):
                PIXEL_WIDTH = size
            else:
                print(f"Width not supported, defaulting to {PIXEL_WIDTH}")
                
        # Specify display height
        if sys.argv[i] == "-h" and i < argc - 1:
            size = int(sys.argv[i + 1])
            if is_supported("height", size):
                PIXEL_HEIGHT = size
            else:
                print(f"Height not supported, defaulting to {PIXEL_HEIGHT}")
                
        # Specify image path
        if sys.argv[i] == "-i" and i < argc - 1:
            IMG_FILE_NAME = sys.argv[i + 1]
                

def main() -> None:
    global IMG_FILE_NAME, PIXEL_WIDTH, PIXEL_HEIGHT
    parse_sysargs()
    
    # Get image data
    print(f"Image file : {IMG_FILE_NAME}")
    img = Image.open(IMG_FILE_NAME, "r")    
    # set resolution / resize
    img_res = img.resize((PIXEL_WIDTH, PIXEL_HEIGHT), Image.LANCZOS)
    
    img_res_data = asarray(img_res)
    print(img_res_data.shape)
        
    
    with open(SOURCE_FILE_RPATH + SOURCE_FILE_NAME, "w")as ofile:
        ofile.write("#include \"st7735.h\"\n")
        ofile.write(SOURCE_FILE_COMMENTS[0])
        ofile.write("const uint8_t frame_buffer[PIXEL_WIDTH * PIXEL_HEIGHT * 3] = {\n")
        
        for i in range(PIXEL_HEIGHT):
            ofile.write("\t")
            for j in range(PIXEL_WIDTH):
                # map color to RGB-666 and write to file
                red = int(img_res_data[i][j][0] * 0xFC / 0xFF)
                green = int(img_res_data[i][j][1] * 0xFC / 0xFF)
                blue = int(img_res_data[i][j][2] * 0xFC / 0xFF)
                ofile.write(f"0x{red:02x}, 0x{green:02x}, 0x{blue:02x},   ")
            ofile.write("\n")
            
        ofile.write("};")

if __name__ == "__main__":
    main()