# How to build 
# Required 
# 1. Original source tarball
# 2. Patches
#
# Build
# Copy tarball and patches to /usr/src/redhat/SOURCES/
# run rpmbuild -ba openradius.spec
# cross fingers and wait
# you could find ready to use rpm in /usr/src/redhat/RPMS/

%define servver 0.9.12c 
%define modver 1.0
Patch0: openradius-0.9.12c.patch
Patch1:openradius-0.9.12c-dublefree.bin
Patch2:openradius-0.9.12c-msgauth.patch

Name:       openradius
Version: 1.0
Release:    3
Summary:    Openradius 
Group:      Openradius 
Vendor:	    Somebody 
License:    GPL
URL:        http://www.xs4all.nl/~evbergen/openradius/ 
Source0:    %{name}-%{servver}.tar.gz
Source1:    %{name}-modules-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-root
%description
OpenRADIUS is built around some design ideas that determine most of how it works; if you build any server using these principles, then a lot of its architecture will already be defined.

# define packages
%package server
Group:  Openradius
Summary: Server related routines
%description  server
Same as above.

# define packages
%package bcgw
Group:  Openradius
Summary: Openradius module 
%description  bcgw
Same as above.

# define packages
%package dbmsdate
Group:  Openradius
Summary: Openradius module 
%description  dbmsdate
Same as above.

# define packages
%package kvfile
Group:  Openradius
Summary: Openradius module 
%description  kvfile
Same as above.

# define packages
%package radacctwriter
Group:  Openradius
Summary: Openradius module 
%description  radacctwriter
Same as above.

# define packages
%package radclasspacker
Group:  Openradius
Summary: Openradius module 
%description  radclasspacker
Same as above.

# define packages
%package radecho
Group:  Openradius
Summary: Openradius module 
%description  radecho
Same as above.

# define packages
%package radeval
Group:  Openradius
Summary: Openradius module 
%description  radeval
Same as above.

# define packages
%package radlogwriter
Group:  Openradius
Summary: Openradius module 
%description  radlogwriter
Same as above.

# define packages
%package snmpgw
Group:  Openradius
Summary: Openradius module 
%description  snmpgw
Same as above.

# define packages
%package absgw
Group:  Openradius
Summary: Openradius module 
%description  absgw
Same as above.

# define packages
%package radre
Group:  Openradius
Summary: Openradius module 
%description  radre
Same as above.

# define packages
%package multi_radclient
Group:  Openradius
Summary: Openradius module 
%description  multi_radclient
Same as above.

%prep
%setup -b0 -n %{name}-%{servver}
%patch0 -p 0
%patch1 -p 0
%patch2 -p 0
%setup -b1 -n %{name}-modules-%{version}

%build
cd ..
cd %{name}-%{servver} 
make
#build modules
cd ..
cd %{name}-modules-%{version}
#
cd absgw 
sh autogen.sh
sh configure --with-openradius-source-dir=../../%{name}-%{servver}/ --prefix=%{buildroot}/usr/local
make
cd ..
#
cd bcgw
sh autogen.sh
sh configure --prefix=%{buildroot}/usr/local
make
# kvfile
cd ..
cd kvfile
sh autogen.sh
sh configure --prefix=%{buildroot}/usr/local
make
# snmpgw
cd ..
cd snmpgw
sh autogen.sh
sh configure --prefix=%{buildroot}/usr/local
make
#

%install
%{__rm} -rf %{buildroot}/*
cd ..
cd %{name}-%{servver}
make install 
make PREFIX=%{buildroot}/usr/local  install
#
cd ..
cd %{name}-modules-%{version}
#
cd absgw
make PREFIX=%{buildroot}/usr/local/lib/openradius install
cd ..
#
cd bcgw
make install
cd ..
#
cd kvfile
make install
cd ..
#
cd snmpgw
make install
cd ..
#
cd radre
sh autogen.sh
sh configure
make DESTDIR=%{buildroot} install
cd ..
#
cd multi_radclient
sh autogen.sh
sh configure
make DESTDIR=%{buildroot} install
cd ..
#
mkdir -p %{buildroot}/var/run/radius
mkdir -p %{buildroot}/var/log/openradius/acct
%{__rm} -r %{buildroot}/usr/local/lib/openradius/*
cp ./dbmsdate/dbmsdate %{buildroot}/usr/local/lib/openradius/
rm ./radacctlogger/radacctlogger
cp ./radacctlogger/radacctwriter %{buildroot}/usr/local/lib/openradius/
cp ./radclasspacker/radclasspacker %{buildroot}/usr/local/lib/openradius/
cp ./radecho/radecho %{buildroot}/usr/local/lib/openradius/
cp ./radeval/radeval %{buildroot}/usr/local/lib/openradius/
rm ./radinfologger/radinfologger
cp ./radinfologger/radlogwriter %{buildroot}/usr/local/lib/openradius/
#

#
%{__rm} -rf %{buildroot}/usr/local/etc/openradius/
%{__rm} -rf %{buildroot}/usr/local/etc/openradius/modules/
%{__rm} %{buildroot}/usr/local/bin/ascenddatafilter
%{__rm} %{buildroot}/usr/local/bin/radtest
%{__rm} %{buildroot}/usr/local/bin/radaccttest
%{__rm} %{buildroot}/usr/local/bin/genmd5hexpasswd
%{__mv} %{buildroot}/usr/local/sbin/radiusd %{buildroot}/usr/local/sbin/openradiusd
%{__mkdir} -p %{buildroot}/usr/local/lib/
%{__mv} %{buildroot}/usr/local/bin/radclient %{buildroot}/usr/local/lib/openradius/radclient
%{__mv} %{buildroot}/usr/local/sbin/bcgw %{buildroot}/usr/local/lib/openradius/bcgw
%{__mv} %{buildroot}/usr/local/sbin/kvfile %{buildroot}/usr/local/lib/openradius/kvfile
%{__mv} %{buildroot}/usr/local/sbin/snmpgw %{buildroot}/usr/local/lib/openradius/snmpgw
%{__mv} %{buildroot}/usr/local/sbin/absgw %{buildroot}/usr/local/lib/openradius/absgw
%{__mv} %{buildroot}/usr/local/sbin/radre %{buildroot}/usr/local/lib/openradius/radre
%{__mv} %{buildroot}/usr/local/sbin/multi_radclient %{buildroot}/usr/local/lib/openradius/multi_radclient
#

#server 
%pre server 
# create users 
/usr/sbin/groupadd radius
/usr/sbin/useradd -c "RADIUS Daemon" \
	-s /sbin/nologin -r -M -g radius radius > /dev/null 2>&1
/sbin/chconfig -add radius > /dev/null 2>&1 || :
#

%post server 
#
if test -e /var/log/openradius/acct ; then
	%{__chown} -R radius:radius /var/log/openradius
fi
#
if test -e /var/run/radius ; then 
	%{__chown} -R radius:radius /var/run/radius
fi
#
%files server 
%defattr(-,root,root)
/usr/local/sbin/openradiusd
/usr/local/lib/openradius/radclient
/var/log/openradius
/var/run/radius
/var/log/openradius/acct
# bcgw
%files bcgw
%defattr(-,root,root)
/usr/local/lib/openradius/bcgw
#kvfile
%files kvfile
%defattr(-,root,root)
/usr/local/lib/openradius/kvfile
# snmpgw
%files snmpgw
%defattr(-,root,root)
/usr/local/lib/openradius/snmpgw
# dbmsdate
%files dbmsdate
%defattr(-,root,root)
/usr/local/lib/openradius/dbmsdate 
# radacctlogger
%files radacctwriter
%defattr(-,root,root)
/usr/local/lib/openradius/radacctwriter
# radclasspacker
%files radclasspacker
%defattr(-,root,root)
/usr/local/lib/openradius/radclasspacker
# radecho
%files radecho
%defattr(-,root,root)
/usr/local/lib/openradius/radecho
# radeval
%files radeval
%defattr(-,root,root)
/usr/local/lib/openradius/radeval
# radlogwriter
%files radlogwriter
%defattr(-,root,root)
/usr/local/lib/openradius/radlogwriter
# absgw
%files absgw
%defattr(-, root, root)
/usr/local/lib/openradius/absgw
# radre
%files radre
%defattr(-, root, root)
/usr/local/lib/openradius/radre
# multi_radclient
%files multi_radclient
%defattr(-, root, root)
/usr/local/lib/openradius/multi_radclient
#
