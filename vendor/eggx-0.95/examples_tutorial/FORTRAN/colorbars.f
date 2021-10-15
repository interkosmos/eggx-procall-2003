c
c     colorbars
c
      parameter( BAR_HEIGHT=8 )
      parameter( BAR_WIDTH=480 )
      integer win,key,i,j
      integer cl_r,cl_g,cl_b
      real width,dat,x,y0,y1 

      call gopen(480,8*59,win)
      call layer(win,0,1)

      do i=0,58
         do j=0,BAR_WIDTH
            width = BAR_WIDTH * 1.0
            dat = j * 1.0
            call makecolor(58-i,0.0,width,dat,cl_r,cl_g,cl_b)
            call newrgbcolor(win,cl_r,cl_g,cl_b)
            x=j * 1.0
            y0=i * BAR_HEIGHT * 1.0
            y1=(i+1) * (BAR_HEIGHT) * 1.0 - 2.0
            call moveto(win,x,y0)
            call lineto(win,x,y1)
         enddo
         call copylayer(win,1,0)
      enddo

      call ggetch(key)
      call gclose(win)
      stop
      end
