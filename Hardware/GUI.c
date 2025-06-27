#include "stm32f10x.h"
#include "Lcd_Driver.h"
#include "GUI.h"
#include "delay.h"
#include "font.h"

//从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。
//通过该函数转换
//c:GBR格式的颜色值
//返回值：RGB格式的颜色值
u16 LCD_BGR2RGB(u16 c)
{
  u16  r,g,b,rgb;   
  b=(c>>0)&0x1f;
  g=(c>>5)&0x3f;
  r=(c>>11)&0x1f;	 
  rgb=(b<<11)+(g<<5)+(r<<0);		 
  return(rgb);

}


/**
  * 函    数：绘制位图
  * 参    数：x - 起始X坐标
  *           y - 起始Y坐标
  *           width - 图像宽度
  *           height - 图像高度
  *           data - 图像数据指针
  * 返 回 值：无
  * 说    明：图像数据格式为RGB565，每个像素占2字节
  */
void Gui_DrawBMP(u16 x, u16 y, u16 width, u16 height, u8 *data)
{
    u16 i, j;
    u16 color;
    u32 index = 0;
    
    for(i = 0; i < height; i++)
    {
        for(j = 0; j < width; j++)
        {
            // 从数据中读取RGB565颜色值（两个字节组成一个像素）
            color = (data[index] << 8) | data[index + 1];
            index += 2;
            
            // 使用已有的绘点函数
            Gui_DrawPoint(x + j, y + i, color);
        }
    }
}

void Gui_Circle(u16 X,u16 Y,u16 R,u16 fc) 
{//Bresenham算法 
    unsigned short  a,b; 
    int c; 
    a=0; 
    b=R; 
    c=3-2*R; 
    while (a<b) 
    { 
        Gui_DrawPoint(X+a,Y+b,fc);     //        7 
        Gui_DrawPoint(X-a,Y+b,fc);     //        6 
        Gui_DrawPoint(X+a,Y-b,fc);     //        2 
        Gui_DrawPoint(X-a,Y-b,fc);     //        3 
        Gui_DrawPoint(X+b,Y+a,fc);     //        8 
        Gui_DrawPoint(X-b,Y+a,fc);     //        5 
        Gui_DrawPoint(X+b,Y-a,fc);     //        1 
        Gui_DrawPoint(X-b,Y-a,fc);     //        4 

        if(c<0) c=c+4*a+6; 
        else 
        { 
            c=c+4*(a-b)+10; 
            b-=1; 
        } 
       a+=1; 
    } 
    if (a==b) 
    { 
        Gui_DrawPoint(X+a,Y+b,fc); 
        Gui_DrawPoint(X+a,Y+b,fc); 
        Gui_DrawPoint(X+a,Y-b,fc); 
        Gui_DrawPoint(X-a,Y-b,fc); 
        Gui_DrawPoint(X+b,Y+a,fc); 
        Gui_DrawPoint(X-b,Y+a,fc); 
        Gui_DrawPoint(X+b,Y-a,fc); 
        Gui_DrawPoint(X-b,Y-a,fc); 
    } 
	
} 
//画线函数，使用Bresenham 画线算法
void Gui_DrawLine(u16 x0, u16 y0,u16 x1, u16 y1,u16 Color)   
{
int dx,             // difference in x's
    dy,             // difference in y's
    dx2,            // dx,dy * 2
    dy2, 
    x_inc,          // amount in pixel space to move during drawing
    y_inc,          // amount in pixel space to move during drawing
    error,          // the discriminant i.e. error i.e. decision variable
    index;          // used for looping	


	Lcd_SetXY(x0,y0);
	dx = x1-x0;//计算x距离
	dy = y1-y0;//计算y距离

	if (dx>=0)
	{
		x_inc = 1;
	}
	else
	{
		x_inc = -1;
		dx    = -dx;  
	} 
	
	if (dy>=0)
	{
		y_inc = 1;
	} 
	else
	{
		y_inc = -1;
		dy    = -dy; 
	} 

	dx2 = dx << 1;
	dy2 = dy << 1;

	if (dx > dy)//x距离大于y距离，那么每个x轴上只有一个点，每个y轴上有若干个点
	{//且线的点数等于x距离，以x轴递增画点
		// initialize error term
		error = dy2 - dx; 

		// draw the line
		for (index=0; index <= dx; index++)//要画的点数不会超过x距离
		{
			//画点
			Gui_DrawPoint(x0,y0,Color);
			
			// test if error has overflowed
			if (error >= 0) //是否需要增加y坐标值
			{
				error-=dx2;

				// move to next line
				y0+=y_inc;//增加y坐标值
			} // end if error overflowed

			// adjust the error term
			error+=dy2;

			// move to the next pixel
			x0+=x_inc;//x坐标值每次画点后都递增1
		} // end for
	} // end if |slope| <= 1
	else//y轴大于x轴，则每个y轴上只有一个点，x轴若干个点
	{//以y轴为递增画点
		// initialize error term
		error = dx2 - dy; 

		// draw the line
		for (index=0; index <= dy; index++)
		{
			// set the pixel
			Gui_DrawPoint(x0,y0,Color);

			// test if error overflowed
			if (error >= 0)
			{
				error-=dy2;

				// move to next line
				x0+=x_inc;
			} // end if error overflowed

			// adjust the error term
			error+=dx2;

			// move to the next pixel
			y0+=y_inc;
		} // end for
	} // end else |slope| > 1
}



void Gui_box(u16 x, u16 y, u16 w, u16 h,u16 bc)
{
	Gui_DrawLine(x,y,x+w,y,0xEF7D);
	Gui_DrawLine(x+w-1,y+1,x+w-1,y+1+h,0x2965);
	Gui_DrawLine(x,y+h,x+w,y+h,0x2965);
	Gui_DrawLine(x,y,x,y+h,0xEF7D);
    Gui_DrawLine(x+1,y+1,x+1+w-2,y+1+h-2,bc);
}
void Gui_box2(u16 x,u16 y,u16 w,u16 h, u8 mode)
{
	if (mode==0)	{
		Gui_DrawLine(x,y,x+w,y,0xEF7D);
		Gui_DrawLine(x+w-1,y+1,x+w-1,y+1+h,0x2965);
		Gui_DrawLine(x,y+h,x+w,y+h,0x2965);
		Gui_DrawLine(x,y,x,y+h,0xEF7D);
		}
	if (mode==1)	{
		Gui_DrawLine(x,y,x+w,y,0x2965);
		Gui_DrawLine(x+w-1,y+1,x+w-1,y+1+h,0xEF7D);
		Gui_DrawLine(x,y+h,x+w,y+h,0xEF7D);
		Gui_DrawLine(x,y,x,y+h,0x2965);
	}
	if (mode==2)	{
		Gui_DrawLine(x,y,x+w,y,0xffff);
		Gui_DrawLine(x+w-1,y+1,x+w-1,y+1+h,0xffff);
		Gui_DrawLine(x,y+h,x+w,y+h,0xffff);
		Gui_DrawLine(x,y,x,y+h,0xffff);
	}
}


/**************************************************************************************
功能描述: 在屏幕显示一凸起的按钮框
输    入: u16 x1,y1,x2,y2 按钮框左上角和右下角坐标
输    出: 无
**************************************************************************************/
void DisplayButtonDown(u16 x1,u16 y1,u16 x2,u16 y2)
{
	Gui_DrawLine(x1,  y1,  x2,y1, GRAY2);  //H
	Gui_DrawLine(x1+1,y1+1,x2,y1+1, GRAY1);  //H
	Gui_DrawLine(x1,  y1,  x1,y2, GRAY2);  //V
	Gui_DrawLine(x1+1,y1+1,x1+1,y2, GRAY1);  //V
	Gui_DrawLine(x1,  y2,  x2,y2, WHITE);  //H
	Gui_DrawLine(x2,  y1,  x2,y2, WHITE);  //V
}

/**************************************************************************************
功能描述: 在屏幕显示一凹下的按钮框
输    入: u16 x1,y1,x2,y2 按钮框左上角和右下角坐标
输    出: 无
**************************************************************************************/
void DisplayButtonUp(u16 x1,u16 y1,u16 x2,u16 y2)
{
	Gui_DrawLine(x1,  y1,  x2,y1, WHITE); //H
	Gui_DrawLine(x1,  y1,  x1,y2, WHITE); //V
	
	Gui_DrawLine(x1+1,y2-1,x2,y2-1, GRAY1);  //H
	Gui_DrawLine(x1,  y2,  x2,y2, GRAY2);  //H
	Gui_DrawLine(x2-1,y1+1,x2-1,y2, GRAY1);  //V
    Gui_DrawLine(x2  ,y1  ,x2,y2, GRAY2); //V
}


void Gui_DrawFont_GBK16(u16 x, u16 y, u16 fc, u16 bc, u8 *s)
{
	unsigned char i,j;
	unsigned short k,x0;
	x0=x;

	while(*s) 
	{	
		if((*s) < 128) 
		{
			k=*s;
			if (k==13) 
			{
				x=x0;
				y+=16;
			}
			else 
			{
				if (k>32) k-=32; else k=0;
	
			    for(i=0;i<16;i++)
				for(j=0;j<8;j++) 
					{
				    	if(asc16[k*16+i]&(0x80>>j))	Gui_DrawPoint(x+j,y+i,fc);
						else 
						{
							if (fc!=bc) Gui_DrawPoint(x+j,y+i,bc);
						}
					}
				x+=8;
			}
			s++;
		}
			
		else 
		{
		

			for (k=0;k<hz16_num;k++) 
			{
			  if ((hz16[k].Index[0]==*(s))&&(hz16[k].Index[1]==*(s+1)))
			  { 
				    for(i=0;i<16;i++)
				    {
						for(j=0;j<8;j++) 
							{
						    	if(hz16[k].Msk[i*2]&(0x80>>j))	Gui_DrawPoint(x+j,y+i,fc);
								else {
									if (fc!=bc) Gui_DrawPoint(x+j,y+i,bc);
								}
							}
						for(j=0;j<8;j++) 
							{
						    	if(hz16[k].Msk[i*2+1]&(0x80>>j))	Gui_DrawPoint(x+j+8,y+i,fc);
								else 
								{
									if (fc!=bc) Gui_DrawPoint(x+j+8,y+i,bc);
								}
							}
				    }
				}
			  }
			s+=2;x+=16;
		} 
		
	}
}

void Gui_DrawFont_GBK24(u16 x, u16 y, u16 fc, u16 bc, u8 *s)
{
	unsigned char i,j;
	unsigned short k;

	while(*s) 
	{
		if( *s < 0x80 ) 
		{
			k=*s;
			if (k>32) k-=32; else k=0;

		    for(i=0;i<16;i++)
			for(j=0;j<8;j++) 
				{
			    	if(asc16[k*16+i]&(0x80>>j))	
					Gui_DrawPoint(x+j,y+i,fc);
					else 
					{
						if (fc!=bc) Gui_DrawPoint(x+j,y+i,bc);
					}
				}
			s++;x+=8;
		}
		else 
		{

			for (k=0;k<hz24_num;k++) 
			{
			  if ((hz24[k].Index[0]==*(s))&&(hz24[k].Index[1]==*(s+1)))
			  { 
				    for(i=0;i<24;i++)
				    {
						for(j=0;j<8;j++) 
							{
						    	if(hz24[k].Msk[i*3]&(0x80>>j))
								Gui_DrawPoint(x+j,y+i,fc);
								else 
								{
									if (fc!=bc) Gui_DrawPoint(x+j,y+i,bc);
								}
							}
						for(j=0;j<8;j++) 
							{
						    	if(hz24[k].Msk[i*3+1]&(0x80>>j))	Gui_DrawPoint(x+j+8,y+i,fc);
								else {
									if (fc!=bc) Gui_DrawPoint(x+j+8,y+i,bc);
								}
							}
						for(j=0;j<8;j++) 
							{
						    	if(hz24[k].Msk[i*3+2]&(0x80>>j))	
								Gui_DrawPoint(x+j+16,y+i,fc);
								else 
								{
									if (fc!=bc) Gui_DrawPoint(x+j+16,y+i,bc);
								}
							}
				    }
			  }
			}
			s+=2;x+=24;
		}
	}
}
void Gui_DrawFont_Num32(u16 x, u16 y, u16 fc, u16 bc, u16 num)
{
	unsigned char i,j,k,c;
	//lcd_text_any(x+94+i*42,y+34,32,32,0x7E8,0x0,sz32,knum[i]);
//	w=w/8;

    for(i=0;i<32;i++)
	{
		for(j=0;j<4;j++) 
		{
			c=*(sz32+num*32*4+i*4+j);
			for (k=0;k<8;k++)	
			{
	
		    	if(c&(0x80>>k))	Gui_DrawPoint(x+j*8+k,y+i,fc);
				else {
					if (fc!=bc) Gui_DrawPoint(x+j*8+k,y+i,bc);
				}
			}
		}
	}
}

// 显示数字函数（非数码管数字）
void LCD_ShowNum(u16 x, u16 y, u16 num, u8 len, u16 fc, u16 bc)
{
    char buffer[16]; // 字符缓冲区
    u8 i = 0, j = 0;
    u16 temp = num;
    
    // 计算数字的位数
    if(num == 0) {
        i = 1; // 如果是0，位数为1
    } else {
        do {
            i++;
            temp /= 10;
        } while(temp);
    }
    
    // 如果显示位数小于指定位数，前面补零
    for(j = 0; j < len - i; j++) {
        buffer[j] = '0';
    }
    
    // 转换数字为字符串
    temp = num;
    if(num == 0) {
        buffer[j] = '0';
        j++;
    } else {
        for(; i > 0; i--) {
            buffer[len - i] = temp % 10 + '0';
            temp /= 10;
        }
    }
    
    buffer[len] = '\0'; // 字符串结束符
    
    // 显示字符串
    Gui_DrawFont_GBK16(x, y, fc, bc, (u8 *)buffer);
}

void LCD_ShowHexNum(u16 x, u16 y, u32 num, u16 fc, u16 bc, u8 length)
{
    // 校验length范围（1~8）
    if (length == 0 || length > 8) length = 8;
    
    // 创建足够大的缓冲区（8位十六进制 + 终止符）
    char buffer[9]; 
    const char hex_table[] = "0123456789ABCDEF"; // 大写字母表

    // 从高位到低位填充字符
    for (u8 i = 0; i < length; i++) 
    {
        // 计算当前4位的位移量（最高位开始）
        u8 shift = (length - i - 1) * 4;
        u8 nibble = (num >> shift) & 0x0F; // 提取4位
        buffer[i] = hex_table[nibble];     // 查表转换字符
    }
    buffer[length] = '\0'; // 终止符

    // 显示字符串（调整间距需在Gui_DrawFont_GBK16内部实现）
    Gui_DrawFont_GBK16(x, y, fc, bc, (u8*)buffer);
}

// 显示更小字体的函数
void Gui_DrawFont_Small(u16 x, u16 y, u16 fc, u16 bc, u8 *s)
{
    unsigned char i,j;
    unsigned short k,x0;
    x0=x;

    while(*s) 
    {    
        if((*s) < 128) 
        {
            k=*s;
            if (k==13) 
            {
                x=x0;
                y+=10;  // 行高减为10像素
            }
            else 
            {
                if (k>32) k-=32; else k=0;
    
                // 只绘制8x10像素而不是8x16像素
                for(i=0;i<10;i++)  // 高度减为10
                for(j=0;j<6;j++)   // 宽度减为6
                {
                    // 使用相同的字库数据，但跳过一些像素
                    if(i < 16 && asc16[k*16+(i*16/10)]&(0x80>>(j*8/6)))
                        Gui_DrawPoint(x+j,y+i,fc);
                    else 
                    {
                        if (fc!=bc) Gui_DrawPoint(x+j,y+i,bc);
                    }
                }
                x+=6;  // 每个字符宽度减为6像素
            }
            s++;
        }
        else 
        {
            // 中文字符处理 - 同样缩小
            for (k=0;k<hz16_num;k++) 
            {
                if ((hz16[k].Index[0]==*(s))&&(hz16[k].Index[1]==*(s+1)))
                { 
                    for(i=0;i<10;i++)  // 高度减为10
                    {
                        for(j=0;j<6;j++) // 宽度部分1减为6
                        {
                            if(hz16[k].Msk[i*2*10/16]&(0x80>>j*8/6))
                                Gui_DrawPoint(x+j,y+i,fc);
                            else 
                            {
                                if (fc!=bc) Gui_DrawPoint(x+j,y+i,bc);
                            }
                        }
                        for(j=0;j<6;j++) // 宽度部分2减为6
                        {
                            if(hz16[k].Msk[i*2*10/16+1]&(0x80>>j*8/6))
                                Gui_DrawPoint(x+j+6,y+i,fc);
                            else 
                            {
                                if (fc!=bc) Gui_DrawPoint(x+j+6,y+i,bc);
                            }
                        }
                    }
                }
            }
            s+=2;
            x+=12;  // 中文字符宽度减为12像素
        } 
    }
}

/**
  * @brief  显示十进制有符号整数
  * @param  x: 起始X坐标（像素）
  * @param  y: 起始Y坐标（像素）
  * @param  num: 要显示的数字（范围：-2147483648~2147483647）
  * @param  len: 显示的数字总长度（包含符号位，如显示"-123"需len=4）
  * @param  fc: 字体颜色
  * @param  bc: 背景颜色
  */
void LCD_ShowSignedNum(u16 x, u16 y, int32_t num, u8 len, u16 fc, u16 bc)
{
    u8 i;
    u32 num_abs;
    char buffer[12]; // 存储符号+数字的字符串
    
    // 处理符号
    if (num >= 0) {
        buffer[0] = '+';
        num_abs = num;
    } else {
        buffer[0] = '-';
        num_abs = -num;
    }
    
    // 显示符号
    Gui_DrawFont_GBK16(x, y, fc, bc, (u8*)&buffer[0]);
    
    // 转换为数字字符串（处理绝对值）
    for (i = 0; i < len-1; i++) { // len-1因为符号占1位
        buffer[len-1 - i] = (num_abs % 10) + '0';
        num_abs /= 10;
    }
    buffer[len] = '\0';
    
    // 显示数字部分（从x+8开始，跳过符号位）
    Gui_DrawFont_GBK16(x + 8, y, fc, bc, (u8*)&buffer[1]);
}

// 辅助函数：计算10的幂（用于数字分解）
static u32 LCD_Pow(u8 base, u8 exponent)
{
    u32 result = 1;
    while (exponent--) {
        result *= base;
    }
    return result;
}


