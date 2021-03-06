%define kmod_name		dresden
%define kmod_driver_version	1.0
%define kmod_rpm_release	1
%define kmod_kernel_version	2.6.18-308.1.1.el5

# with koji ... we have to build against current (newest) kernel ...

%{!?dist: %define dist .slc5}

Source0:	%{kmod_name}-%{kmod_driver_version}.tgz
Source1:	kmodtool-%{kmod_name}
Source2:	%{kmod_name}.modules

%define kmodtool	bash %{SOURCE1}
#define __find_requires %_sourcedir/find-requires.ksyms

%define kverrel		%(%{kmodtool} verrel %kmod_kernel_version 2>/dev/null)

# does not build for xen or PAE kernels:
# probes.c:81: error: implicit declaration of function 'regs_return_value'
# jarek, 16.03.2012
#ifarch i686 x86_64 ia64
#define xenvar xen
#endif
#ifarch i686
#define paevar PAE
#endif

%define upvar	""
%{!?kvariants: %define kvariants %{?upvar} %{?xenvar} %{?paevar}}

Name:		%{kmod_name}
Version:	%{kmod_driver_version}
Release:	%{kmod_rpm_release}%{?dist}
Summary:	Kernel module for logging network connections details
Group:		System Environment/Kernel
License:	GPLv2+
URL:		http://www.cern.ch/
Vendor:		CERN, http://cern.ch/linux
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
BuildRequires:	sed
BuildRequires:	%kernel_module_package_buildreqs
ExclusiveArch:	i686 x86_64

%description
%{kmod_name} is a Loadable Kernel Module that logs information for every connection.

# magic hidden here:
%{expand:%(%{kmodtool} rpmtemplate_kmp %{kmod_name} %{kverrel} %{kvariants} 2>/dev/null)}

# Define the filter.
%define _use_internal_dependency_generator	0
%define __find_requires				sh %{_builddir}/%{buildsubdir}/filter-requires.sh

%prep
# to understand the magic better or to debug it, uncomment this:
#%{kmodtool} rpmtemplate_kmp %{kmod_name} %{kverrel} %{kvariants} 2>/dev/null
#sleep 15
%setup -q -c -T -a 0

# let's pretend it is kABI whitelisted only ...
echo "/usr/lib/rpm/redhat/find-requires | %{__sed} -e '/^ksym.*/d'" > filter-requires.sh

for kvariant in %{kvariants} ; do
	cp -a %{kmod_name}-%{version} _kmod_build_$kvariant
done

%build
echo  %{kverrel}
for kvariant in %{kvariants}
do
	ksrc=%{_usrsrc}/kernels/%{kverrel}${kvariant:+-$kvariant}-%{_target_cpu}
	pushd _kmod_build_$kvariant

	# update symvers file if existing
	symvers=Module.symvers${kvariant:+-$kvariant}-%{_target_cpu}
	if [ -e $symvers ]; then
		cp $symvers Module.symvers
	fi

	make -C "${ksrc}" modules M=$PWD
	popd
done

if [ -d firmware ]; then
	make -C firmware
fi

%install
rm -rf $RPM_BUILD_ROOT
export INSTALL_MOD_PATH=$RPM_BUILD_ROOT
export INSTALL_MOD_DIR=extra/%{kmod_name}
for kvariant in %{kvariants}
do
	ksrc=%{_usrsrc}/kernels/%{kverrel}${kvariant:+-$kvariant}-%{_target_cpu}
	pushd _kmod_build_$kvariant
	make -C "${ksrc}" modules_install M=$PWD
	popd

	 # create depmod configuration
	 mkdir -p ${RPM_BUILD_ROOT}/etc/depmod.d
	 echo "override %{kmod_name} * weak-updates/%{kmod_name}" > ${RPM_BUILD_ROOT}/etc/depmod.d/%{kmod_name}${kvariant}.conf
done

mkdir -p ${RPM_BUILD_ROOT}/lib/firmware

if [ -d firmware ]; then
	make -C firmware INSTALL_PATH=$RPM_BUILD_ROOT INSTALL_DIR=updates install
fi

#Load at boot time

mkdir -p ${RPM_BUILD_ROOT}/etc/sysconfig/modules/
install -m0755 %{SOURCE2} ${RPM_BUILD_ROOT}/etc/sysconfig/modules/

%post

${RPM_BUILD_ROOT}/etc/sysconfig/modules/%{kmod_name}.modules

%clean
rm -rf $RPM_BUILD_ROOT

%changelog
* Fri Jul 6 2012 Panos Sakkos <panos.sakkos@cern.ch> - 1.0
- Initial pachkaging.