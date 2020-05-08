#include <ssulib.h>
#include <device/console.h>
#include <device/io.h>
#include <syscall.h>


int strncmp(char* b1, char* b2, int len)
{
	int i;

	for(i = 0; i < len; i++)
	{
		char c = b1[i] - b2[i];
		if(c)
			return c;
		if(b1[i] == 0)
			return 0;
	}
	return 0;
}

bool getkbd(char *buf, int len) 
{
	char ch;
	int offset = 0;

	len--;

	for(; offset < len && buf[offset] != 0; offset++)
		if(buf[offset] == '\n')
		{
			for(;offset>=0;offset--)
				buf[offset] = 0;
			offset++;
			break;
		}

	while ( (ch = ssuread()) >= 0)
	{
		if(ch == '\b' && offset == 0)
		{
			set_cursor();
			continue;
		}
		printk("%c",ch);
		set_cursor();
		if (ch == '\b')
		{
			buf[offset-1] = 0;
			offset--;
		}
		else if (ch == '\n')
		{
			buf[offset] = ch;
			return FALSE;
		}
		else
		{
			buf[offset] = ch;
			offset++;
		}
		if(offset == len) offset--;
	}
	
	return TRUE;
}

int getToken(char* buf, char token[][BUFSIZ], int max_tok)
{
	int num_tok = 0;
	int off_tok = 0;
	while(num_tok < max_tok && *buf != '\n')
	{
		if(*buf == ' ') 
		{
			token[num_tok][off_tok] = 0;
			while(*buf == ' ') buf++;
			off_tok = 0;
			num_tok++;
		}
		else
		{
			token[num_tok][off_tok] = *buf;
			off_tok++;
			buf++;
		}
	}
	token[num_tok][off_tok] = 0;
	num_tok++;

	return num_tok;
}
