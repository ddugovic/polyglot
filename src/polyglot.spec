Summary: A Winboard protocol to UCI protocol adapter
Name: polyglot
Version: 2.01
Release: 1
License: GPL
Group: Amusement/Games
URL: http://hgm.nubati.net/cgi-bin/gitweb.cgi

Source: http://hgm.nubati.net/polyglot/polyglot-%{version}.tar.gz
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
* Sat Apr 18 2012 H.G.Muller <h.g.muller@hccnet.nl> - 2.0.1
- Start of new fork
* Sat Jan 3 2009 Michel Van den Bergh <michel.vandenbergh@uhasselt.be> - 1.4w10UCIb10-1
- Initial spec file

