#include <stdio.h>

#define ROWS 30
#define COLS 50

char canvas[ROWS][COLS];

void clearCanvas()
{
    int i,j;
    for(i=0;i<ROWS;i++)
    {
        for(j=0;j<COLS;j++)
        {
            canvas[i][j]=' ';
        }
    }
}

void displayCanvas()
{
    int i,j;
    for(i=0;i<ROWS;i++)
    {
        for(j=0;j<COLS;j++)
        {
            printf("%c",canvas[i][j]);
        }
        printf("\n");
    }
}

void drawRectangle(int x,int y,int w,int h)
{
    int i;

    for(i=x;i<x+w;i++)
    {
        canvas[y][i]='*';
        canvas[y+h-1][i]='*';
    }

    for(i=y;i<y+h;i++)
    {
        canvas[i][x]='*';
        canvas[i][x+w-1]='*';
    }
}

void drawLine(int x1,int y1,int x2,int y2)
{
    int i;

    if(y1==y2)
    {
        for(i=x1;i<=x2;i++)
            canvas[y1][i]='*';
    }
    else if(x1==x2)
    {
        for(i=y1;i<=y2;i++)
            canvas[i][x1]='*';
    }
}

void drawTriangle(int x,int y,int size)
{
    int i;

    for(i=0;i<size;i++)
    {
        canvas[y+i][x]='*';
        canvas[y+i][x+i]='*';
    }

    for(i=0;i<size;i++)
    {
        canvas[y+size-1][x+i]='*';
    }
}

void drawCircle(int cx,int cy,int r)
{
    int x,y;

    for(y=0;y<ROWS;y++)
    {
        for(x=0;x<COLS;x++)
        {
            int dx=x-cx;
            int dy=y-cy;

            if(dx*dx + dy*dy >= r*r-r &&
               dx*dx + dy*dy <= r*r+r)
            {
                canvas[y][x]='*';
            }
        }
    }
}

void deleteArea(int x,int y,int w,int h)
{
    int i,j;

    for(i=y;i<y+h;i++)
    {
        for(j=x;j<x+w;j++)
        {
            canvas[i][j]=' ';
        }
    }
}

int main()
{
    int choice;

    clearCanvas();

    while(1)
    {
        printf("\n===== 2D GRAPHICS EDITOR =====\n");
        printf("1. Draw Rectangle\n");
        printf("2. Draw Line\n");
        printf("3. Draw Triangle\n");
        printf("4. Draw Circle\n");
        printf("5. Delete Area\n");
        printf("6. Display Picture\n");
        printf("7. Exit\n");

        printf("Enter choice: ");
        scanf("%d",&choice);

        if(choice==1)
        {
            int x,y,w,h;
            printf("x y width height: ");
            scanf("%d%d%d%d",&x,&y,&w,&h);
            drawRectangle(x,y,w,h);
        }
        else if(choice==2)
        {
            int x1,y1,x2,y2;
            printf("x1 y1 x2 y2: ");
            scanf("%d%d%d%d",&x1,&y1,&x2,&y2);
            drawLine(x1,y1,x2,y2);
        }
        else if(choice==3)
        {
            int x,y,s;
            printf("x y size: ");
            scanf("%d%d%d",&x,&y,&s);
            drawTriangle(x,y,s);
        }
        else if(choice==4)
        {
            int x,y,r;
            printf("centerX centerY radius: ");
            scanf("%d%d%d",&x,&y,&r);
            drawCircle(x,y,r);
        }
        else if(choice==5)
        {
            int x,y,w,h;
            printf("x y width height: ");
            scanf("%d%d%d%d",&x,&y,&w,&h);
            deleteArea(x,y,w,h);
        }
        else if(choice==6)
        {
            displayCanvas();
        }
        else if(choice==7)
        {
            break;
        }
    }

    return 0;
}
