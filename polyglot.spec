Summary: A Winboard protocol to UCI protocol adapter
Name: polyglot
Version: 1.4w10UCIb22
Release: 1
License: GPL
Group: Amusement/Games
URL: http://alpha.uhasselt.be/Research/Algebra/Toga

Source: http://alpha.uhasselt.be/Research/Algebra/Toga/polyglot-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

%description
PolyGlot is a "UCI adapter".  It connects a GUI interface (such as
 XBoard, Winboard, Arena or Chessbase) to a UCI chess engine.

%prep
%setup

%build
%configure 
%{__make}

%install
%{__rm} -rf %{buildroot}
%makeinstall

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-, root, root, 0755)
%doc %{_mandir}/man6/polyglot.6* 
%doc %{_docdir}/polyglot/README*
%doc %{_docdir}/polyglot/book_format.html
%{_bindir}/polyglot

%changelog
* Sat Jan 3 2009 Michel Van den Bergh <michel.vandenbergh@uhasselt.be> - 1.4w10UCIb10-1
- Initial spec file

