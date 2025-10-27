#include <videoDriver.h>

struct vbe_mode_info_structure {
	uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8_t window_a;			// deprecated
	uint8_t window_b;			// deprecated
	uint16_t granularity;		// deprecated; used while calculating bank numbers
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	uint16_t pitch;			// number of bytes per horizontal line
	uint16_t width;			// width in pixels
	uint16_t height;			// height in pixels
	uint8_t w_char;			// unused...
	uint8_t y_char;			// ...
	uint8_t planes;
	uint8_t bpp;			// bits per pixel in this mode
	uint8_t banks;			// deprecated; total number of banks in this mode
	uint8_t memory_model;
	uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	uint8_t image_pages;
	uint8_t reserved0;
 
	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;
 
	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8_t reserved1[206];
} __attribute__ ((packed));

typedef struct vbe_mode_info_structure * VBEInfoPtr;

VBEInfoPtr VBE_mode_info = (VBEInfoPtr) 0x0000000000005C00;

void putPixel(uint32_t hexColor, uint64_t x, uint64_t y) {
    uint8_t * framebuffer = (uint8_t *) VBE_mode_info->framebuffer;
	/**
	 videoDriver.c: In function 'putPixel':
	 videoDriver.c:46:29: warning: cast to pointer from integer of different size [-Wint-to-pointer-cast]
   		46 |     uint8_t * framebuffer = (uint8_t *) VBE_mode_info->framebuffer;
	 */
    uint64_t offset = (x * ((VBE_mode_info->bpp)/8)) + (y * VBE_mode_info->pitch);
    framebuffer[offset]     =  (hexColor) & 0xFF;
    framebuffer[offset+1]   =  (hexColor >> 8) & 0xFF; 
    framebuffer[offset+2]   =  (hexColor >> 16) & 0xFF;
}

void clearScreen(uint32_t color) {
    for (uint64_t y = 0; y < VBE_mode_info->height; y++) {
        for (uint64_t x = 0; x < VBE_mode_info->width; x++) {
            putPixel(color, x, y);
        }
    } 
}

uint64_t getWidth(){
	return VBE_mode_info->width;
}

uint64_t getHeight(){
	return VBE_mode_info->height;
}


static char isPenDown = 0;
void penUp(){
	isPenDown = 0;
}
void penDown(){
	isPenDown = 1;
}

uint64_t x_pos = 0;
uint64_t y_pos = 0;
uint32_t currHexColor = 0x74FF00;

void setColor(uint32_t color){
	currHexColor = color;
}

int left(uint64_t delta){
	int i;
	for(i = x_pos; i > 0 && i > x_pos-delta; i--){
		if(isPenDown){
			putPixel(currHexColor,i,y_pos);
		}
	}
	x_pos = i;
	return i-(x_pos-delta);
}
int right(uint64_t delta){
	int i;
	for(i = x_pos; i < VBE_mode_info->width && i < x_pos+delta; i++){
		if(isPenDown){
			putPixel(currHexColor,i,y_pos);
		}
	}
	x_pos = i;
	return x_pos+delta-i;
}
int up(uint64_t delta){
	int j;
	for(j = y_pos; j > 0 && j > y_pos-delta; j--){
		if (isPenDown){
			putPixel(currHexColor,x_pos,j);
		}
	}
	y_pos = j;
	return y_pos-(y_pos-delta);
}
int down(uint64_t delta){
	int j;
	for(j = y_pos; j < VBE_mode_info->height && j < y_pos+delta; j++){
		if(isPenDown){
			putPixel(currHexColor,x_pos,j);
		}
	
	}
	y_pos = j;
	return y_pos+delta-y_pos;
}

int setX(uint64_t newX){
	if(newX > VBE_mode_info->width || newX < 0){
		return -1;
	}
	x_pos = newX;
	return 0;
}
int setY(uint64_t newY){

	if(newY > VBE_mode_info->height || newY < 0){
		return -1;
	}
	y_pos = newY;
	return 0;
}

int drawSquare(uint64_t x, uint64_t y, uint64_t sideLength, uint32_t hexColor){
	if(x > VBE_mode_info->width || x < 0 || y < 0 || y > VBE_mode_info->height || sideLength <= 0){
		return -1; // Error de argumentos.
	}
	int i = x, j = y;
	for(; i < x+sideLength && i < VBE_mode_info->width; i++){
		for(j = y; j < y+sideLength && j < VBE_mode_info->height; j++ ){
			putPixel(hexColor,i,j);
		}
	}
	return (sideLength * sideLength) - ((i - x) * (j - y)); // Retorna la cantidad de pixeles no dibujados.
}

int drawRectangle(uint64_t x, uint64_t y, uint64_t vLength, uint64_t hLength, uint32_t hexColor){
	for(int i = x; i < hLength + x; i++){
		for (int j = y; j < vLength + y; j++){
			putPixel(hexColor,i,j);
		}
	}
	return 0;
}
