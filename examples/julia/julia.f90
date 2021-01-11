! julia.f90
!
! Julia set demo for EGGX/ProCALL.
!
! Author:  Philipp Engel
! Licence: ISC
program main
    use, intrinsic :: iso_c_binding, only: c_null_char
    use :: procall
    implicit none

    character(len=*), parameter :: WIN_TITLE  = 'Fortran Julia Set'
    integer,          parameter :: WIN_WIDTH  = 800
    integer,          parameter :: WIN_HEIGHT = 600
    integer,          parameter :: MAX_ITER   = 255

    integer :: key
    integer :: j
    integer :: r, g, b
    integer :: win
    integer :: x, y

    call gopen(WIN_WIDTH, WIN_HEIGHT, win)          ! Open X11 window.
    call winname(win, WIN_TITLE // c_null_char)     ! Set window title.
    call gsetbgcolor(win, 'black' // c_null_char)   ! Set background colour.
    call gclr(win)                                  ! Clear drawing area.

    do y = 0, WIN_HEIGHT - 1
        do x = 0, WIN_WIDTH - 1
            j  = julia(x, y, WIN_WIDTH, WIN_HEIGHT, 1.0, 0.0, 0.0, cmplx(-0.7, 0.27015), MAX_ITER)
            call makecolor(PROCALL_IDL1_STERN_SPECIAL, 0.0, real(MAX_ITER), real(j), r, g, b)
            call newrgbcolor(win, r, g, b)
            call pset(win, real(x), real(y))
        end do
    end do

    call ggetch(key)
    call gcloseall()
contains
    function julia(x, y, width, height, zoom, mx, my, c, max_iter)
        integer, intent(in) :: x
        integer, intent(in) :: y
        integer, intent(in) :: width
        integer, intent(in) :: height
        real,    intent(in) :: zoom
        real,    intent(in) :: mx
        real,    intent(in) :: my
        complex, intent(in) :: c
        integer, intent(in) :: max_iter
        integer             :: julia
        integer             :: i
        complex             :: z

        julia = 0
        z = cmplx(1.5 * (x - width / 2) / (0.5 * zoom * width) + mx, &
                  1.0 * (y - height / 2) / (0.5 * zoom * height) + my)

        do i = 1, max_iter
            z = z**2 + c

            if (abs(z) > 2) then
                julia = i
                exit
            end if
        end do
    end function julia
end program main
