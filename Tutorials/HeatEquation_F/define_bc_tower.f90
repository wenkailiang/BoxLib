module define_bc_module

  use bl_types
  use ml_layout_module
  use bc_module

  implicit none

  type bc_level

     integer   :: dim    = 0
     integer   :: ngrids = 0
     type(box) :: domain 
     integer, pointer :: phys_bc_level_array(:,:,:) => Null()
     integer, pointer ::  adv_bc_level_array(:,:,:,:) => Null()

  end type bc_level

  type bc_tower

     integer :: dim     = 0
     integer :: nlevels = 0
     integer :: max_level_built = 0
     type(bc_level), pointer :: bc_tower_array(:) => Null()
     integer       , pointer :: domain_bc(:,:) => Null()

  end type bc_tower

  private

  public :: bc_level, bc_tower, bc_tower_init, bc_tower_level_build, bc_tower_destroy

  contains

  subroutine bc_tower_init(bct,num_levs,dm,phys_bc_in)

    implicit none

    type(bc_tower ), intent(  out) :: bct
    integer        , intent(in   ) :: num_levs
    integer        , intent(in   ) :: dm
    integer        , intent(in   ) :: phys_bc_in(:,:)

    integer :: n

    bct%nlevels = num_levs
    bct%dim     = dm
    allocate(bct%bc_tower_array(bct%nlevels))
    allocate(bct%domain_bc(dm,2))

    do n = 1, num_levs
      bct%bc_tower_array(n)%ngrids = -1
    end do

    bct%domain_bc(:,:) = phys_bc_in(:,:)

  end subroutine bc_tower_init

  subroutine bc_tower_level_build(bct,n,la)

    type(bc_tower ), intent(inout) :: bct
    integer        , intent(in   ) :: n
    type(layout)   , intent(in   ) :: la

    integer :: ngrids
    integer :: default_value

    if (bct%bc_tower_array(n)%ngrids > 0) then
      deallocate(bct%bc_tower_array(n)%phys_bc_level_array)
      deallocate(bct%bc_tower_array(n)%adv_bc_level_array)
    end if

    ngrids = layout_nboxes(la)
    bct%bc_tower_array(n)%dim    = bct%dim
    bct%bc_tower_array(n)%ngrids = ngrids
    bct%bc_tower_array(n)%domain = layout_get_pd(la)

    allocate(bct%bc_tower_array(n)%phys_bc_level_array(0:ngrids,bct%dim,2))
    default_value = INTERIOR
    call phys_bc_level_build(bct%bc_tower_array(n)%phys_bc_level_array,la, &
                             bct%domain_bc,default_value)

    ! Here we allocate 1 component and set the default to be interior
    allocate(bct%bc_tower_array(n)%adv_bc_level_array(0:ngrids,bct%dim,2,1))
    default_value = INTERIOR
    call adv_bc_level_build(bct%bc_tower_array(n)%adv_bc_level_array, &
                            bct%bc_tower_array(n)%phys_bc_level_array,default_value)

     bct%max_level_built = n

  end subroutine bc_tower_level_build

  subroutine bc_tower_destroy(bct)

    implicit none

    type(bc_tower), intent(inout) :: bct

    integer :: n

    do n = 1,bct%max_level_built
       deallocate(bct%bc_tower_array(n)%phys_bc_level_array)
       deallocate(bct%bc_tower_array(n)%adv_bc_level_array)
    end do
    deallocate(bct%bc_tower_array)

    deallocate(bct%domain_bc)

  end subroutine bc_tower_destroy

  subroutine phys_bc_level_build(phys_bc_level,la_level,domain_bc,default_value)

    implicit none

    integer     , intent(inout) :: phys_bc_level(0:,:,:)
    integer     , intent(in   ) :: domain_bc(:,:)
    type(layout), intent(in   ) :: la_level
    integer     , intent(in   ) :: default_value
    type(box) :: bx,pd
    integer :: d,i

    pd = layout_get_pd(la_level) 

    phys_bc_level = default_value

    i = 0
    do d = 1,layout_dim(la_level)
       phys_bc_level(i,d,1) = domain_bc(d,1)
       phys_bc_level(i,d,2) = domain_bc(d,2)
    end do

    do i = 1,layout_nboxes(la_level)
       bx = layout_get_box(la_level,i)
       do d = 1,layout_dim(la_level)
          if (lwb(bx,d) == lwb(pd,d)) phys_bc_level(i,d,1) = domain_bc(d,1)
          if (upb(bx,d) == upb(pd,d)) phys_bc_level(i,d,2) = domain_bc(d,2)
       end do
    end do

  end subroutine phys_bc_level_build

  subroutine adv_bc_level_build(adv_bc_level,phys_bc_level,default_value)

    implicit none

    integer  , intent(inout) ::  adv_bc_level(0:,:,:,:)
    integer  , intent(in   ) :: phys_bc_level(0:,:,:)
    integer  , intent(in   ) :: default_value

    integer :: dm
    integer :: igrid,d,lohi

    adv_bc_level = default_value
    print *,'DEFAULT ',default_value

    dm = size(adv_bc_level,dim=2)

    do igrid = 0, size(adv_bc_level,dim=1)-1
    do d = 1, dm
    do lohi = 1, 2

       if (phys_bc_level(igrid,d,lohi) == OUTLET) then

          adv_bc_level(igrid,d,lohi,1) = FOEXTRAP

       else if (phys_bc_level(igrid,d,lohi) == SYMMETRY) then

          adv_bc_level(igrid,d,lohi,1) = REFLECT_EVEN

       end if

    end do
    end do
    end do

  end subroutine adv_bc_level_build

end module define_bc_module