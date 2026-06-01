import yaml
import csv
import os

def section_filename(title):
    return f"{title.replace('(', '').replace(')', '').replace(' ', '_').replace('/', '_')}.csv"

def generate_md(data):
    md_lines = []
    md_lines.append(f"# {data.get('title', 'Tailoring Guide')}\n")
    if 'intro' in data:
        md_lines.append(f"{data['intro']}\n")

    for sec in data['sections']:
        md_lines.append(f"## {sec['title']}\n")
        if 'text' in sec:
            md_lines.append(f"{sec['text']}\n")
            
        if 'groups' in sec:
            for group in sec['groups']:
                req = f" ({group['requirement']})" if 'requirement' in group else ""
                group_name = group.get('group_name', group.get('cif_bit', 'General'))
                md_lines.append(f"### {group_name}{req}")
                
                if 'notes' in group and group['notes']:
                    md_lines.append(f"**Notes:** {group['notes']}\n")
                
                if group['fields']:
                    # Determine columns to show
                    # Default known columns in order
                    possible_cols = [
                        'Word', 'Bit', 'Field Bit', 'Designation', 'Bit Used', 'Bits Used', 
                        'Default Value', 'Range', 'Function', 'Time Determined', 
                        'Time When Payload Value Determined', 'Field Size (words)', 'Notes'
                    ]
                    
                    active_cols = []
                    for col in possible_cols:
                        if any(col in f for f in group['fields']):
                            active_cols.append(col)
                    
                    # Add any extra columns not in the known list
                    for f in group['fields']:
                        for k in f.keys():
                            if k not in active_cols:
                                active_cols.append(k)

                    if active_cols:
                        md_lines.append("| " + " | ".join(active_cols) + " |")
                        md_lines.append("|" + "|".join(["---"] * len(active_cols)) + "|")
                        for f in group['fields']:
                            row_str = "| " + " | ".join([str(f.get(col, '')).replace('\n', ' <br> ') for col in active_cols]) + " |"
                            md_lines.append(row_str)
                md_lines.append("\n")
        elif 'rows' in sec and sec['rows']:
            headers = sec.get('headers', [])
            if headers:
                md_lines.append("| " + " | ".join(headers) + " |")
                md_lines.append("|" + "|".join(["---"] * len(headers)) + "|")
                for row in sec['rows']:
                    row_str = "| " + " | ".join([str(row.get(col, '')).replace('\n', ' <br> ') for col in headers]) + " |"
                    md_lines.append(row_str)
            md_lines.append("\n")

    with open('spec/generated/ams_vita_49-2_tailoring.md', 'w') as f:
        f.write('\n'.join(md_lines))

def generate_csv(data):
    if not os.path.exists('spec/generated/csv'):
        os.makedirs('spec/generated/csv')

    for sec in data['sections']:
        if 'groups' in sec:
            # We want to export a CSV for this packet/section
            filename = f"spec/generated/csv/{section_filename(sec['title'])}"
            
            # Determine all columns
            possible_cols = [
                'Group/Word/CIF', 'Requirement', 'Group Notes', 'Word', 'Bit', 'Field Bit', 'Designation', 
                'Bit Used', 'Bits Used', 'Default Value', 'Range', 'Function', 'Time Determined', 
                'Time When Payload Value Determined', 'Field Size (words)', 'Notes'
            ]
            
            # check which are actually used
            active_cols = ['Group/Word/CIF', 'Requirement', 'Group Notes']
            for col in possible_cols[3:]:
                for group in sec['groups']:
                    if any(col in f for f in group['fields']):
                        if col not in active_cols:
                            active_cols.append(col)
                            
            # Any remaining unknown cols
            for group in sec['groups']:
                for f in group['fields']:
                    for k in f.keys():
                        if k not in active_cols:
                            active_cols.append(k)
            
            if 'Bit' not in active_cols and any('Bit' in f for g in sec['groups'] for f in g['fields']):
                print("Bit is in fields but not active_cols?!", sec['title'])
                
            with open(filename, 'w', newline='') as f:
                writer = csv.DictWriter(f, fieldnames=active_cols, lineterminator='\n')
                writer.writeheader()
                for group in sec['groups']:
                    group_name = group.get('group_name', group.get('cif_bit', 'General'))
                    req = group.get('requirement', '')
                    group_notes = group.get('notes', '')
                    
                    for field in group['fields']:
                        row = {'Group/Word/CIF': group_name, 'Requirement': req, 'Group Notes': group_notes}
                        row.update(field)
                        try:
                            writer.writerow(row)
                        except ValueError as e:
                            print(f"Error on section {sec['title']}, group {group_name}")
                            print("active_cols:", active_cols)
                            print("row keys:", row.keys())
                            raise e
        
        elif 'rows' in sec and sec['rows']:
            filename = f"spec/generated/csv/{section_filename(sec['title'])}"
            headers = sec.get('headers', [])
            if headers:
                with open(filename, 'w', newline='') as f:
                    writer = csv.DictWriter(f, fieldnames=headers, lineterminator='\n')
                    writer.writeheader()
                    for row in sec['rows']:
                        writer.writerow(row)

if __name__ == '__main__':
    with open('spec/ams_vita_49-2_tailoring.yaml', 'r') as f:
        data = yaml.safe_load(f)
    
    generate_md(data)
    generate_csv(data)
