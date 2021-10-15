c
      integer win,win_in,b
      integer type
      real x,y

      x=0
      y=0

      call gopen(400,300,win)
      call gsetbgcolor(win,'blue4'//CHAR(0))
      call gclr(win)

      write(*,*) 'Click the window or hit any key'

      do
         call ggetxpress(win_in,type,b,x,y)
         if ( win_in .eq. win ) then
            if ( type .eq. 4 ) then
               call newpencolor(win,b)
               call moveto(win,0.0,y)
               call lineto(win,399.0,y)
               call moveto(win,x,0.0)
               call lineto(win,x,299.0)
               call drawnum(win,x,y, 14.0, x ,0.0,0)
               call drawnum(win,x+7*5.0,y, 14.0, y ,0.0,0)
               write(*,*) 'button = ',b
               write(*,*) 'x,y =',x,y
            else if ( type .eq. 2 ) then
               if ( b .eq.  z'71' ) then
                  exit
               endif
               write(*,*) 'key code = ',b
            endif
         endif
      enddo
      call gclose(win)
      
      end
c
