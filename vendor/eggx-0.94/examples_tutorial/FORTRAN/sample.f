c
c     procallのテスト
c
      integer i,key
c      integer depth
      real xs,ys,h,hh
      xs=640.0
      ys=400.0
c     Windowを開く
      call plots
c      call winname(0,'procall-test'//CHAR(0))
c      call clsc
      call clsx
      do 100 i=1,10
         h=i*16
         hh=i*8
         call newpen( i )
         call plot( hh,hh,3 )
         call plot( xs-hh,hh,2 )
         call plot( xs-hh,ys-hh,2 )
         call plot( hh,ys-hh,2 )
         call plot( hh,hh,2 )
         call symbol( 96+h*2,96+h,16.0,i,0,-1)
         call drawstr( 0, 96+h*2+48,96+h,i*2.0+4,'Pro',0.0,3 )
 100  continue
c      call saveimg(0,0,0.0,0.0,639.0,
c     &     399.0,'img.gif'//CHAR(0),68,'ppmtogif'//CHAR(0),16)
      call ggetch(key)
c     Windowを閉じる
      call gcloseall()
      stop
      end
