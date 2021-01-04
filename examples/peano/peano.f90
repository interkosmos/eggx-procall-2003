! peano.f90
!
! Demo application that plots a Peano curve with ProCALL.
!
! Author:  Philipp Engel
! Licence: ISC
program main
    use, intrinsic :: iso_c_binding, only: c_null_char
    use :: procall
    implicit none
    integer :: key
    integer :: win

    call gopen(800, 800, win)
    call winname(win, 'Peano Curve' // c_null_char)

    call gsetbgcolor(win, 'black' // c_null_char)
    call newpencolor(win, 3)
    call gclr(win)

    call moveto(win, 0.0, 0.0)
    call peano(win, 0, 0, 1000, 0, 0)

    call ggetch(key)
    call gcloseall()
contains
    recursive subroutine peano(win, x, y, lg, i1, i2)
        integer, intent(in) :: win
        integer, intent(in) :: x
        integer, intent(in) :: y
        integer, intent(in) :: lg
        integer, intent(in) :: i1
        integer, intent(in) :: i2
        integer             :: l

        if (lg == 1) then
            call lineto(win, real(3 * x), real(3 * y))
            return
        end if

        l = lg / 3

        call peano(win,        (2 * i1 * l) + x,        (2 * i1 * l) + y, l,     i1,     i2)
        call peano(win, ((i1 - i2 + 1) * l) + x,     ((i1 + i2) * l) + y, l,     i1, 1 - i2)
        call peano(win,                   l + x,                   l + y, l,     i1, 1 - i2)
        call peano(win,     ((i1 + i2) * l) + x, ((i1 - i2 + 1) * l) + y, l, 1 - i1, 1 - i2)
        call peano(win,        (2 * i2 * l) + x,  (2 * (1 - i2) * l) + y, l,     i1,     i2)
        call peano(win, ((1 + i2 - i1) * l) + x, ((2 - i1 - i2) * l) + y, l,     i1,     i2)
        call peano(win,  (2 * (1 - i1) * l) + x,  (2 * (1 - i1) * l) + y, l,     i1,     i2)
        call peano(win, ((2 - i1 - i2) * l) + x, ((1 + i2 - i1) * l) + y, l, 1 - i1,     i2)
        call peano(win,  (2 * (1 - i2) * l) + x,        (2 * i2 * l) + y, l, 1 - i1,     i2)
    end subroutine peano
end program main
