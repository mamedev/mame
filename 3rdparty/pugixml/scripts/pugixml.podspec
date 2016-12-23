Pod::Spec.new do |s|
  s.name         = "pugixml"
  s.version      = "1.7"
  s.summary      = "C++ XML parser library."
  s.homepage     = "http://pugixml.org"
  s.license      = { :type => 'MIT', :text => <<-qwertyuiop
The MIT License (MIT)

Copyright (c) 2006-2016 Arseny Kapoulkine

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
qwertyuiop
}
  s.author       = { "Arseny Kapoulkine" => "arseny.kapoulkine@gmail.com" }
  s.platform     = :ios, "7.0"
  
  s.source = { :git => "https://github.com/zeux/pugixml.git", :tag => "v" + s.version.to_s }

  s.source_files  = "src/**/*.{hpp,cpp}"
  s.header_mappings_dir = "src"
end
