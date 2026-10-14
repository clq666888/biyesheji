#include "gpio.h"
#include "OLED_Font.h"

/* OLED显存缓存：按页存储，1页对应屏幕上的8行像素 */
static uint8_t OLED_DisplayBuffer[8][128] = {{0}};
static uint8_t OLED_CurrentPage = 0;
static uint8_t OLED_CurrentColumn = 0;
/* 记录各字符格当前是否处于反相状态，供OLED_Blackback统一恢复 */
static uint8_t OLED_ReverseState[4][16] = {{0}};

/*引脚配置*/
//#define OLED_W_SCL(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x))
//#define OLED_W_SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(x))
void OLED_W_SCL(GPIO_PinState x){
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8,x);
}
void OLED_W_SDA(GPIO_PinState x){
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9,x);
}

/*引脚初始化*/
void OLED_I2C_Init(void)
{
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//
//	GPIO_InitTypeDef GPIO_InitStructure;
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	 MX_GPIO_Init();

	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
  * @brief  I2C开始
  * @param  无
  * @retval 无
  */
void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_W_SDA(0);
	OLED_W_SCL(0);
}

/**
  * @brief  I2C停止
  * @param  无
  * @retval 无
  */
void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
  * @brief  I2C发送一个字节
  * @param  Byte 要发送的一个字节
  * @retval 无
  */
void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		OLED_W_SDA(Byte & (0x80 >> i));
		OLED_W_SCL(1);
		OLED_W_SCL(0);
	}
	OLED_W_SCL(1);	//额外的一个时钟，不处理应答信号
	OLED_W_SCL(0);
}

/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令
  * @retval 无
  */
void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x00);		//写命令
	OLED_I2C_SendByte(Command); 
	OLED_I2C_Stop();
}

/**
  * @brief  OLED写数据
  * @param  Data 要写入的数据
  * @retval 无
  */
void OLED_WriteData(uint8_t Data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x40);		//写数据
	OLED_I2C_SendByte(Data);
	OLED_I2C_Stop();

	/* 同步软件显存，供OLED_ReverseArea读取和修改 */
	if (OLED_CurrentPage < 8 && OLED_CurrentColumn < 128)
	{
		OLED_DisplayBuffer[OLED_CurrentPage][OLED_CurrentColumn] = Data;
		OLED_CurrentColumn++;
	}
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  * @retval 无
  */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	/* 记录当前显存位置，OLED_WriteData据此更新软件显存 */
	if (Y < 8 && X < 128)
	{
		OLED_CurrentPage = Y;
		OLED_CurrentColumn = X;
	}
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}

/**
  * @brief  OLED清屏
  * @param  无
  * @retval 无
  */
void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < 8; j++)
	{
		OLED_SetCursor(j, 0);
		for(i = 0; i < 128; i++)
		{
			OLED_WriteData(0x00);
		}
	}

	/* 清屏后显存全黑，所有字符格均不再处于反相状态 */
	for (j = 0; j < 4; j++)
	{
		for (i = 0; i < 16; i++)
		{
			OLED_ReverseState[j][i] = 0;
		}
	}
}

/**
  * @brief  反相OLED指定区域（按字符格选择，单格大小为8×16像素）
  * @param  Line 行位置，范围：0~4；1~4选择指定行，0表示所有行
  * @param  Column 列位置，范围：0~16；1~16选择指定列，0表示所有列
  * @retval 无；Line或Column超出范围时不执行操作
  */
void OLED_ReverseArea(uint8_t Line, uint8_t Column)
{
	uint8_t CellLine, CellColumn;
	uint8_t PageStart, PageEnd;
	uint8_t ColumnStart, ColumnEnd;
	uint8_t Page, X;

	if (Line > 4 || Column > 16)
	{
		return;
	}

	/* 计算目标区域对应的页（每页8像素高）和列（每字符格8像素宽） */
	if (Line == 0)
	{
		PageStart = 0;
		PageEnd = 8;
	}
	else
	{
		PageStart = (Line - 1) * 2;
		PageEnd = PageStart + 2;
	}

	if (Column == 0)
	{
		ColumnStart = 0;
		ColumnEnd = 128;
	}
	else
	{
		ColumnStart = (Column - 1) * 8;
		ColumnEnd = ColumnStart + 8;
	}

	/* 按位取反当前显存内容，并立即写回OLED */
	for (Page = PageStart; Page < PageEnd; Page++)
	{
		OLED_SetCursor(Page, ColumnStart);
		for (X = ColumnStart; X < ColumnEnd; X++)
		{
			OLED_WriteData((uint8_t)~OLED_DisplayBuffer[Page][X]);
		}
	}

	/* 同步记录被切换的字符格，供OLED_Blackback恢复 */
	for (CellLine = 1; CellLine <= 4; CellLine++)
	{
		if (Line == 0 || Line == CellLine)
		{
			for (CellColumn = 1; CellColumn <= 16; CellColumn++)
			{
				if (Column == 0 || Column == CellColumn)
				{
					OLED_ReverseState[CellLine - 1][CellColumn - 1] ^= 1;
				}
			}
		}
	}
}


/**
  * @brief  取消所有通过OLED_ReverseArea设置的反相，保留屏幕原有内容
  * @param  无
  * @retval 无
  */
void OLED_Blackback(void)
{
	uint8_t Line, Column;
	for (Line = 1; Line <= 4; Line++)
	{
		for (Column = 1; Column <= 16; Column++)
		{
			if (OLED_ReverseState[Line - 1][Column - 1])
			{
				OLED_ReverseArea(Line, Column);
			}
		}
	}
}

/**
  * @brief  OLED显示一个字符
  * @param  Line 行位置，范围：1~4
  * @param  Column 列位置，范围：1~16
  * @param  Char 要显示的一个字符，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
	uint8_t i;
	uint8_t Data;
	uint8_t Reverse;

	if (Line < 1 || Line > 4 || Column < 1 || Column > 16)
	{
		return;
	}

	Reverse = OLED_ReverseState[Line - 1][Column - 1];
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
	for (i = 0; i < 8; i++)
	{
		Data = OLED_F8x16[Char - ' '][i];			//取上半部分字模
		if (Reverse)
		{
			Data = (uint8_t)~Data;			//该字格处于反相时，写入反色字模
		}
		OLED_WriteData(Data);
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分
	for (i = 0; i < 8; i++)
	{
		Data = OLED_F8x16[Char - ' '][i + 8];	//取下半部分字模
		if (Reverse)
		{
			Data = (uint8_t)~Data;			//该字格处于反相时，写入反色字模
		}
		OLED_WriteData(Data);
	}
}

/**
  * @brief  OLED显示字符串
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  String 要显示的字符串，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}

/**
  * @brief  OLED次方函数
  * @retval 返回值等于X的Y次方
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

/**
  * @brief  OLED显示数字（十进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~4294967295
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十进制，带符号数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：-2147483648~2147483647
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)
	{
		OLED_ShowChar(Line, Column, '+');
		Number1 = Number;
	}
	else
	{
		OLED_ShowChar(Line, Column, '-');
		Number1 = -Number;
	}
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十六进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~0xFFFFFFFF
  * @param  Length 要显示数字的长度，范围：1~8
  * @retval 无
  */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)							
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10)
		{
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}
		else
		{
			OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
		}
	}
}

/**
  * @brief  OLED显示数字（二进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~1111 1111 1111 1111
  * @param  Length 要显示数字的长度，范围：1~16
  * @retval 无
  */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}

/**
  * @brief  OLED初始化
  * @param  无
  * @retval 无
  */
void OLED_Init(void)
{
	uint32_t i, j;
	
	for (i = 0; i < 1000; i++)			//上电延时
	{
		for (j = 0; j < 1000; j++);
	}
	
	OLED_I2C_Init();			//端口初始化
	
	OLED_WriteCommand(0xAE);	//关闭显示
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//设置显示开始行
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度控制
	OLED_WriteCommand(0xCF);

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示
		
	OLED_Clear();				//OLED清屏
}
