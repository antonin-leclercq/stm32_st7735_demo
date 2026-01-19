import sys
import os
from PIL import Image
from numpy import asarray

SOURCE_FILE_RPATH = "../app/data/"
SOURCE_FILE_NAME = None
PIXEL_MAX_WIDTH = 128
PIXEL_MAX_HEIGHT = 160

HELP = "usage : python frame_gen.py [options]\n" \
        "with options being :\n" \
        "\t-w <frame width> : set frame width in pixel (default 128)\n" \
        "\t-h <frame height> : set frame height in pixel (default 160) \n" \
        "\t-i <path> : fill frame with data from image located at <path>\n" \
        "\t-f <format> : set RGB format(for example 444, 565 or 666)\n"\
        "\t--help : display this help message\n"

def is_supported(width_height: str, size: int) -> bool:
    if width_height == "width":
        return size > 0 and size <= PIXEL_MAX_WIDTH
    elif width_height == "height":
        return size > 0 and size <= PIXEL_MAX_HEIGHT
    return False
    

def parse_sysargs() -> tuple:
    IMG_FILE_NAME = ""
    PIXEL_WIDTH = PIXEL_MAX_WIDTH
    PIXEL_HEIGHT = PIXEL_MAX_HEIGHT
    argc = len(sys.argv)
    if argc == 1:
        print("No arguments specified")
        print(HELP)
        sys.exit(0)
        
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
            
    return IMG_FILE_NAME, PIXEL_WIDTH, PIXEL_HEIGHT
                

def main() -> None:
    IMG_FILE_NAME, PIXEL_WIDTH, PIXEL_HEIGHT = parse_sysargs()
    
    # Get image data
    print(f"Image file : {IMG_FILE_NAME}")
    img = Image.open(IMG_FILE_NAME, "r")
    # set resolution / resize
    img_res = img.resize((PIXEL_WIDTH, PIXEL_HEIGHT), Image.LANCZOS)
    
    img_res_data = asarray(img_res)
    print(img_res_data.shape)
    
    img_name_only = os.path.basename(IMG_FILE_NAME).split('.')[0]
    
    HEADER_FILE_NAME = img_name_only.lower() + "_frame.h"
    BUFFER_NAME = img_name_only.lower() + "_buffer"
    
    def_width = img_name_only.upper() + "_WIDTH"
    def_height = img_name_only.upper() + "_HEIGHT"
    
    with open(SOURCE_FILE_RPATH + HEADER_FILE_NAME, "w") as hfile:
        def1 = HEADER_FILE_NAME.upper().replace(".", "_")
        hfile.write(f"#ifndef {def1}\n")
        hfile.write(f"#define {def1}\n\n")
        
        hfile.write(f"#define {def_width} {PIXEL_WIDTH}\n")
        hfile.write(f"#define {def_height} {PIXEL_HEIGHT}\n\n")
        
        hfile.write("typedef unsigned char uint8_t;\n\n")
        
        hfile.write(f"static const uint8_t {BUFFER_NAME}[{def_width} * {def_height} * 3] = {{\n")
        
        for i in range(PIXEL_HEIGHT):
            hfile.write("\t")
            for j in range(PIXEL_WIDTH):
                # map color to RGB-666 and write to file
                red = int(img_res_data[i][j][0] * 0xFC / 0xFF)
                green = int(img_res_data[i][j][1] * 0xFC / 0xFF)
                blue = int(img_res_data[i][j][2] * 0xFC / 0xFF)
                hfile.write(f"0x{red:02x}, 0x{green:02x}, 0x{blue:02x},   ")
            hfile.write("\n")
            
        hfile.write("};\n\n")

        hfile.write("#endif")

if __name__ == "__main__":
    main()