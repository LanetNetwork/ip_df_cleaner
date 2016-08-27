Name:           ip_df_cleaner
Version:        0.0.1
Release:        1%{?dist}
Summary:        NFQUEUE userspace helper to clean IP DF bit

License:        GPLv3
URL:            https://github.com/LanetNetwork/ip_df_cleaner
Source0:        ip_df_cleaner-0.0.1.tar.gz

BuildRequires:   gcc cmake make gperftools-devel libnetfilter_queue-devel
Requires:        gperftools-devel libnetfilter_queue
Requires(post):  systemd-units
Requires(preun): systemd-units

%description
NFQUEUE userspace helper to clean IP DF bit

%prep
%setup -q

%build
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=%{buildroot}%{_prefix} ..
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%{__install} -D -m0644 configs/%{name}@.service %{buildroot}%{_unitdir}/%{name}@.service
cd build
make install

%clean
rm -rf %{buildroot}

%post
%systemd_post %{name}@.service

%preun
%systemd_preun %{name}@.service

%files
%defattr(0644, root, root, 0755)
%doc COPYING README.md
%attr(0755, root, root) %{_bindir}/%{name}
%{_unitdir}/%{name}@.service

%changelog

