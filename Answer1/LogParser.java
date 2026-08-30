import java.io.*;
import java.util.*;

public class LogParser {

    private static final Set<String> VALID_TYPES = Set.of("error", "warning", "info", "debug");

    public static void main(String[] args) {
        if (args.length < 1) {
            System.err.println("Usage: java LogParser <filepath> [lines] [type1,type2,...]");
            System.exit(1);
        }

        String filePath = args[0];
        int numLines = args.length >= 2 ? Integer.parseInt(args[1]) : 10;
        Set<String> types = new HashSet<>();

        if (args.length >= 3) {
            for (String t : args[2].split(",")) {
                String trimmed = t.trim().toLowerCase();
                if (!VALID_TYPES.contains(trimmed))
                    throw new IllegalArgumentException("Invalid log type: " + trimmed);
                types.add(trimmed);
            }
        } else {
            types.add("error");
        }

        File file = new File(filePath);
        if (!file.exists() || !file.isFile())
            throw new IllegalArgumentException("Invalid file path: " + filePath);

        List<String> allLines;
        try {
            allLines = new ArrayList<>();
            try (BufferedReader br = new BufferedReader(new FileReader(file))) {
                String line;
                while ((line = br.readLine()) != null)
                    allLines.add(line);
            }
        } catch (IOException e) {
            throw new RuntimeException("Error reading file: " + e.getMessage());
        }

        List<String> matched = new ArrayList<>();
        for (int i = allLines.size() - 1; i >= 0 && matched.size() < numLines; i--) {
            String line = allLines.get(i);
            for (String type : types) {
                if (line.toLowerCase().startsWith("[" + type + "]")) {
                    matched.add(line);
                    break;
                }
            }
        }

        Collections.reverse(matched);
        for (String line : matched)
            System.out.println(line);
    }
}
