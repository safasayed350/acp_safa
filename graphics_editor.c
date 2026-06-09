#include <stdio.h>

#define ROWS 30
#define COLS 50

char canvas[ROWS][COLS];

void clearCanvas()
{
    int i, j;
    for(i=0;i<ROWS;i++)
    {
        for(j=0;j<COLS;j++)
        {
            canvas[i][j] = ' ';
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
            printf("%c", canvas[i][j]);
        }
        printf("\n");
    }
}

void drawRectangle(int x,int y,int w,int h)
{
    int i;

    for(i=x;i<x+w;i++)
    {
        if(y>=0 && y<ROWS && i>=0 && i<COLS)
            canvas[y][i]='*';

        if(y+h-1>=0 && y+h-1<ROWS && i>=0 && i<COLS)
            canvas[y+h-1][i]='*';
    }

    for(i=y;i<y+h;i++)
    {
        if(i>=0 && i<ROWS && x>=0 && x<COLS)
            canvas[i][x]='*';

        if(i>=0 && i<ROWS && x+w-1>=0 && x+w-1<COLS)
            canvas[i][x+w-1]='*';
    }
}

void drawLine(int x1,int y1,int x2,int y2)
{
    int i;

    if(y1==y2)
    {
        if(x1>x2)
        {
            int t=x1;
            x1=x2;
            x2=t;
        }

        for(i=x1;i<=x2;i++)
        {
            if(i>=0 && i<COLS && y1>=0 && y1<ROWS)
                canvas[y1][i]='*';
        }
    }
    else if(x1==x2)
    {
        if(y1>y2)
        {
            int t=y1;
            y1=y2;
            y2=t;
        }

        for(i=y1;i<=y2;i++)
        {
            if(i>=0 && i<ROWS && x1>=0 && x1<COLS)
                canvas[i][x1]='*';
        }
    }
}

void drawTriangle(int x,int y,int size)
{
    int i;

    for(i=0;i<size;i++)
    {
        if(y+i<ROWS && x<COLS)
            canvas[y+i][x]='*';

        if(y+i<ROWS && x+i<COLS)
            canvas[y+i][x+i]='*';
    }

    for(i=0;i<size;i++)
    {
        if(y+size-1<ROWS && x+i<COLS)
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
            if(i>=0 && i<ROWS && j>=0 && j<COLS)
                canvas[i][j]=' ';
        }
    }
}

void modifyRectangle()
{
    int oldx, oldy, w, h;
    int newx, newy;

    printf("Old x y width height: ");
    scanf("%d%d%d%d",&oldx,&oldy,&w,&h);

    deleteArea(oldx,oldy,w,h);

    printf("New x y: ");
    scanf("%d%d",&newx,&newy);

    drawRectangle(newx,newy,w,h);

    printf("Rectangle modified successfully.\n");
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
        printf("6. Modify Rectangle\n");
        printf("7. Display Picture\n");
        printf("8. Exit\n");

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
            modifyRectangle();
        }
        else if(choice==7)
        {
            displayCanvas();
        }
        else if(choice==8)
        {
            break;
        }
        else
        {
            printf("Invalid choice.\n");
        }
    }

    return 0;
}