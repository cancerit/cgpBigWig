#!/usr/bin/env cwl-runner

cwlVersion: v1.0
class: CommandLineTool
id: "cgpbigwig-bwcat"
label: "cgpbigwig-bwcat"

doc: |
  ![build_status](https://quay.io/repository/wtsicgp/dockstore-cgpbigwig/status)

  A wrapper for the cgpBigWig bwcat tool.

  bwcat documentation can be found [here](https://github.com/cancerit/cgpBigWig#bwcat)

  In order to run the example found in `example/bwcat.json` please download the relevant reference files
  listed in the [`README`](https://github.com/cancerit/cgpdockstore-cgpbigwig/README.md#Example_reference_files)

  See the [dockstore-cgpbigwig](https://github.com/cancerit/dockstore-cgpbigwig)
  website for more information about this wrapper.

  For queries relating to the underlying software see [cgpBigWig](https://github.com/cancerit/cgpBigWig).

requirements:
  - class: DockerRequirement
    dockerPull: "quay.io/wtsicgp/dockstore-cgpbigwig:3.0.1"

baseCommand: bwcat

inputs:
  input_path:
    type: File
    doc: "Path to the input bigwig file"
    inputBinding:
      prefix: --input-path
      position: 1
  include_na:
    type: boolean?
    doc: "Include NA regions in output"
    inputBinding:
      prefix: --include-na
  region:
    type: string?
    doc: "Region or bw file to print to screen format. NB start should be 0 based: (contig:start-stop)"
    inputBinding:
      prefix: --region

outputs:
  output:
    type: stdout

$schemas:
  - https://schema.org/version/latest/schema.rdf

$namespaces:
  s: http://schema.org/

s:codeRepository: https://github.com/cancerit/dockstore-cgpbigwig
s:license: https://spdx.org/licenses/GPL-3.0

s:author:
  - class: s:Person
    s:identifier: https://orcid.org/0000-0002-0407-0386
    s:email: mailto:drj@sanger.ac.uk
    s:name: David Jones

dct:creator:
  "@id": "https://orcid.org/0000-0002-0407-0386"
  foaf:name: David Jones
  foaf:mbox: "drj@sanger.ac.uk"
