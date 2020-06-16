#!/usr/bin/env cwl-runner

doc: |
  ![build_status](https://quay.io/repository/wtsicgp/dockstore-cgpbigwig/status)

  A wrapper for the cgpBigWig bwjoin tool.

  bwjoin documentation can be found [here](https://github.com/cancerit/cgpBigWig#bwjoin)

  In order to run the example found in `example/bwjoin.json` please download the relevant reference files
  listed in the [`README`](https://github.com/cancerit/cgpdockstore-cgpbigwig/README.md#Example_reference_files)

  See the [dockstore-cgpbigwig](https://github.com/cancerit/dockstore-cgpbigwig)
  website for more information about this wrapper.

  For queries relating to the underlying software see [cgpBigWig](https://github.com/cancerit/cgpBigWig).

cwlVersion: v1.0

class: CommandLineTool

id: "cgpbigwig-bwjoin"
label: "cgpbigwig-bwjoin"

baseCommand: bwjoin

requirements:
  - class: DockerRequirement
    dockerPull: "quay.io/wtsicgp/dockstore-cgpbigwig:3.0.1"

inputs:
  input_path:
    type: Directory
    inputBinding:
      prefix: --input-path
      position: 2
  output_path:
    type: File?
    inputBinding:
      prefix: --outfile
  ignore_contigs:
    type: string?
    inputBinding:
      prefix: --ignore-contigs
  ref_index:
    type: File
    inputBinding:
      prefix: --fasta-index
      position: 1

outputs:
  output:
    type: stdout

$namespaces:
    s: http://schema.org/

$schemas:
    - https://schema.org/version/latest/schema.rdf

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
