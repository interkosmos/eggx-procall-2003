! mandelbrot.f90
!
! Mandelbrot demo for EGGX/ProCALL.
!
! Author:  Philipp Engel
! Licence: ISC
program main
    use, intrinsic :: iso_c_binding, only: c_null_char
    use :: procall
    implicit none

    character(len=*), parameter :: WIN_TITLE  = 'Fortran Mandelbrot'
    integer,          parameter :: WIN_WIDTH  = 640
    integer,          parameter :: WIN_HEIGHT = 480
    integer,          parameter :: MAX_ITER   = 80
    real,             parameter :: THRESHOLD  = 2.0

    integer :: key
    integer :: m
    integer :: r, g, b
    integer :: win
    integer :: x, y
    real    :: re, im

    call gopen(WIN_WIDTH, WIN_HEIGHT, win)          ! Open X11 window.
    call winname(win, WIN_TITLE // c_null_char)     ! Set window title.
    call gsetbgcolor(win, 'black' // c_null_char)   ! Set background colour.
    call gclr(win)                                  ! Clear drawing area.

    do y = 0, WIN_HEIGHT - 1
        im = -1.5 + real(y) * 3.0 / real(WIN_HEIGHT)

        do x = 0, WIN_WIDTH - 1
            re = -2.0 + real(x) * 3.0 / real(WIN_WIDTH)
            m  = mandelbrot(cmplx(re, im), MAX_ITER, THRESHOLD)

            call makecolor(PROCALL_IDL1_PRISM, real(MAX_ITER), 0.0, real(m), r, g, b)
            call newrgbcolor(win, r, g, b)
            call pset(win, real(x), real(y))
        end do
    end do

    call ggetch(key)
    call gcloseall()
contains
    function mandelbrot(c, max_iter, threshold)
        complex, intent(in) :: c
        integer, intent(in) :: max_iter
        real,    intent(in) :: threshold
        integer             :: mandelbrot
        complex             :: z

        z = (0.0, 0.0)

        do mandelbrot = 0, max_iter
            z = z**2 + c

            if (abs(z) > threshold) &
                exit
        end do
    end function mandelbrot
end program main
