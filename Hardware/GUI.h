


u16 LCD_BGR2RGB(u16 c);
void Gui_Circle(u16 X,u16 Y,u16 R,u16 fc); 
void Gui_DrawLine(u16 x0, u16 y0,u16 x1, u16 y1,u16 Color);  
void Gui_box(u16 x, u16 y, u16 w, u16 h,u16 bc);
void Gui_box2(u16 x,u16 y,u16 w,u16 h, u8 mode);
void DisplayButtonDown(u16 x1,u16 y1,u16 x2,u16 y2);
void DisplayButtonUp(u16 x1,u16 y1,u16 x2,u16 y2);
void Gui_DrawFont_GBK16(u16 x, u16 y, u16 fc, u16 bc, u8 *s);
void Gui_DrawFont_GBK24(u16 x, u16 y, u16 fc, u16 bc, u8 *s);
void Gui_DrawFont_Num32(u16 x, u16 y, u16 fc, u16 bc, u16 num) ;
void LCD_ShowNum(u16 x, u16 y, u16 num, u8 len, u16 fc, u16 bc);
void LCD_ShowHexNum(u16 x, u16 y, u32 num, u16 fc, u16 bc, u8 length);
void Gui_DrawFont_Small(u16 x, u16 y, u16 fc, u16 bc, u8 *s);
void LCD_ShowSignedNum(u16 x, u16 y, int32_t num, u8 len, u16 fc, u16 bc);
// 添加到GUI.h文件中的函数声明部分
void Gui_DrawBMP(u16 x, u16 y, u16 width, u16 height, u8 *data);

