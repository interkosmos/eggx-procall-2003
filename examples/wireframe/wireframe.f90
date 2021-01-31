! wireframe.f90
!
! Renders a wireframe model of the Tie-Figher with EGGX/ProCALL (you'll need
! some imagination ...).
!
! Author:  Philipp Engel
! Licence: ISC
module render
    implicit none
    private

    type, public :: vertex_type
        real :: x
        real :: y
        real :: z
    end type vertex_type

    type, public :: edge_type
        integer :: v1
        integer :: v2
    end type edge_type

    type, public :: point_type
        real :: x
        real :: y
    end type point_type

    public :: project
    public :: rad
    public :: rotate_x
    public :: rotate_y
    public :: rotate_z
contains
    function project(v, width, height, fov, distance) result(point)
        !! Transforms a 3D vector to a 2D vector by using perspective projection.
        type(vertex_type), intent(inout) :: v
        integer,           intent(in)    :: width
        integer,           intent(in)    :: height
        real,              intent(in)    :: fov
        real,              intent(in)    :: distance
        type(point_type)                 :: point
        real                             :: f

        f = fov / (distance + v%z)

        point%x = v%x * f + width / 2.0
        point%y = -1.0 * v%y * f + height / 2.0
    end function project

    function rad(deg)
        !! Converts an angle from deg to rad.
        real, intent(in) :: deg
        real             :: rad

        rad = deg * acos(-1.0) / 180.
    end function rad

    function rotate_x(v, angle) result(r)
        !! Rotates vector in x.
        type(vertex_type), intent(inout) :: v
        real,              intent(in)    :: angle
        type(vertex_type)                :: r

        r%x = v%x
        r%y = v%y * cos(angle) - v%z * sin(angle)
        r%z = v%z * sin(angle) + v%z * cos(angle)
    end function rotate_x

    function rotate_y(v, angle) result(r)
        !! Rotates vector in y.
        type(vertex_type), intent(inout) :: v
        real,              intent(in)    :: angle
        type(vertex_type)                :: r

        r%x = v%z * sin(angle) + v%x * cos(angle)
        r%y = v%y
        r%z = v%z * cos(angle) - v%x * sin(angle)
    end function rotate_y

    function rotate_z(v, angle) result(r)
        !! Rotates vector in z.
        type(vertex_type), intent(inout) :: v
        real,              intent(in)    :: angle
        type(vertex_type)                :: r

        r%x = v%x * cos(angle) - v%y * sin(angle)
        r%y = v%x * sin(angle) - v%y * cos(angle)
        r%z = v%z
    end function rotate_z
end module render

program main
    use, intrinsic :: iso_c_binding, only: c_null_char
    use :: procall
    use :: render
    implicit none

    character(len=*), parameter :: OBJ_FILE = 'tie.obj'
    integer,          parameter :: WIN_WIDTH  = 640
    integer,          parameter :: WIN_HEIGHT = 480

    integer :: key
    integer :: rc
    integer :: win
    real    :: d, x, y, z

    type(edge_type),   allocatable :: edges(:)
    type(vertex_type), allocatable :: vertices(:)
    type(point_type),  allocatable :: points(:)

    ! Load vertices and edges from Wavefront OBJ file, initialise `points`
    ! array.
    call init(vertices, edges, points, OBJ_FILE, rc)

    if (rc /= 0) then
        print '(3a)', 'Error: loading file "', OBJ_FILE, '" failed'
        stop
    end if

    ! Initialise ProCALL.
    call gopen(WIN_WIDTH, WIN_HEIGHT, win)                  ! Open X11 window.
    call winname(win, 'Fortran Wireframe' // c_null_char)   ! Set window title.
    call gsetbgcolor(win, 'black' // c_null_char)           ! Set background colour.
    call gsetnonblock(1)                                    ! Enable non-blocking input.
    call layer(win, 0, 1)                                   ! Enable double buffering.

    ! Rotation angles in x, y, z, and camera distance to object.
    x = 0.0; y = 0.0; z = 0.0
    d = 4.0

    do
        call ggetch(key)
        if (key == PROCALL_KEY_ESCAPE) exit

        y = y + rad(1.0) ! Rotate in y.
        call update(vertices, points, WIN_WIDTH, WIN_HEIGHT, x, y, z, d)
        call render(win, points, edges)
        call msleep(20)
    end do

    call gcloseall()
contains
    subroutine init(vertices, edges, points, file_path, stat)
        type(vertex_type), allocatable, intent(out)           :: vertices(:)
        type(edge_type),   allocatable, intent(out)           :: edges(:)
        type(point_type),  allocatable, intent(out)           :: points(:)
        character(len=*),               intent(in)            :: file_path
        integer,                        intent(out), optional :: stat
        integer                                               :: rc
        logical                                               :: file_exists

        if (present(stat)) stat = 0

        inquire (file=trim(file_path), exist=file_exists)

        if (.not. file_exists) then
            if (present(stat)) stat = -1
            return
        end if

        call load(trim(file_path), vertices, edges, rc)

        if (rc /= 0) then
            if (present(stat)) stat = rc
            return
        end if

        allocate (points(size(vertices)))
    end subroutine init

    subroutine load(file_path, vertices, edges, stat)
        !! Loads and parses a Wavefront OBJ file.
        character(len=*),               intent(in)            :: file_path
        type(vertex_type), allocatable, intent(inout)         :: vertices(:)
        type(edge_type),   allocatable, intent(inout)         :: edges(:)
        integer,                        intent(out), optional :: stat
        character(len=80)                                     :: buffer
        integer                                               :: fu, i, j, rc
        integer                                               :: nedges, nvertices

        ! Open file for reading.
        open (action='read', file=trim(file_path), newunit=fu, iostat=rc)

        if (rc /= 0) then
            if (present(stat)) stat = rc
            return
        end if

        ! Count number of edges/vertices in file.
        nedges    = 0
        nvertices = 0

        do
            read (fu, '(a)', iostat=rc) buffer
            if (rc /= 0) exit

            if (buffer(1:1) == 'f') nedges = nedges + 1
            if (buffer(1:1) == 'v') nvertices = nvertices + 1
        end do

        if (nedges == 0 .or. nvertices == 0) then
            close (fu)
            if (present(stat)) stat = -1
            return
        end if

        ! Allocate edge and vertex arrays.
        if (allocated(edges)) deallocate (edges)
        allocate (edges (nedges))

        if (allocated(vertices)) deallocate (vertices)
        allocate (vertices(nvertices))

        ! Read edges and vertices into arrays.
        rewind (fu)

        i = 1
        j = 1

        do
            read (fu, '(a)', iostat=rc) buffer
            if (rc /= 0) exit

            select case (buffer(1:1))
                case ('f')
                    read (buffer(2:), *, iostat=rc) edges(i)
                    if (rc /= 0) cycle
                    i = i + 1

                case ('v')
                    read (buffer(2:), *, iostat=rc) vertices(j)
                    if (rc /= 0) cycle
                    j = j + 1

                case default
                    cycle
            end select
        end do

        close (fu)
        if (present(stat)) stat = 0
    end subroutine load

    subroutine update(vertices, points, width, height, x, y, z, distance)
        !! Rotates the 3D object.
        type(vertex_type), intent(inout) :: vertices(:)
        type(point_type),  intent(inout) :: points(:)
        integer,           intent(in)    :: width
        integer,           intent(in)    :: height
        real,              intent(in)    :: x
        real,              intent(in)    :: y
        real,              intent(in)    :: z
        real,              intent(in)    :: distance
        integer                          :: i
        type(vertex_type)                :: v

        do i = 1, size(vertices, 1)
            v = vertices(i)
            v = rotate_x(v, x)
            v = rotate_y(v, y)
            v = rotate_z(v, z)
            points(i) = project(v, width, height, 256.0, distance)
        end do
    end subroutine update

    subroutine render(win, points, edges)
        !! Renders the scene on the layer and copies layer to screen.
        integer,          intent(inout) :: win
        type(point_type), intent(inout) :: points(:)
        type(edge_type),  intent(inout) :: edges(:)
        real                            :: x1, y1, x2, y2
        integer                         :: i

        call gclr(win)
        call newpencolor(win, PROCALL_CYAN)

        do i = 1, size(edges)
            x1 = points(edges(i)%v1)%x
            y1 = points(edges(i)%v1)%y
            x2 = points(edges(i)%v2)%x
            y2 = points(edges(i)%v2)%y

            call moveto(win, x1, y1)
            call lineto(win, x2, y2)
        end do

        call copylayer(win, 1, 0)
    end subroutine render
end program main
