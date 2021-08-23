! eggx.f90
!
! Fortran 2003 ISO C binding interfaces to the EGGX graphics library.
! For more information on EGGX/ProCALL, see:
!
!   https://www.ir.isas.jaxa.jp/~cyamauch/eggx_procall/
!
! Author:  Philipp Engel
! Licence: ISC
module eggx
    use, intrinsic :: iso_c_binding
    implicit none
    private

    public :: eggx_gputimage
    public :: eggx_winname

    interface
        ! int eggx_gputimage(int win, double x, double y, unsigned char *buf, int width, int height, int mask)
        function eggx_gputimage(nwin, x, y, buf, nw, nh, mask) bind(c, name='eggx_gputimage')
            import :: c_char, c_double, c_int
            integer(kind=c_int),    intent(in), value :: nwin
            real(kind=c_double),    intent(in), value :: x
            real(kind=c_double),    intent(in), value :: y
            character(kind=c_char), intent(in)        :: buf(*)
            integer(kind=c_int),    intent(in), value :: nw
            integer(kind=c_int),    intent(in), value :: nh
            integer(kind=c_int),    intent(in), value :: mask
            integer(kind=c_int)                       :: eggx_gputimage
        end function eggx_gputimage

        ! int winname(int wn, const char *argsformat, ...)
        function eggx_winname(nwin, argsformat) bind(c, name='eggx_winname')
            import :: c_char, c_int
            integer(kind=c_int),    intent(in), value :: nwin
            character(kind=c_char), intent(in)        :: argsformat
            integer(kind=c_int)                       :: eggx_winname
        end function eggx_winname
    end interface
end module eggx
